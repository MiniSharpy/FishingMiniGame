// Fill out your copyright notice in the Description page of Project Settings.


#include "Fisher.h"
#include "Fish.h"
#include "Kismet/KismetMathLibrary.h"

//TODO: Put contents of switch statements in methods for readibility.

// Sets default values
AFisher::AFisher()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Fisher"));

	CastWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("CastWidget"));
	CastWidget->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFisher::BeginPlay()
{
	Super::BeginPlay();

	//PlayerController = GetController<APlayerController>(); //Controllers don't exist until game is play is started.
	PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) //Ensuring player controller exists.
	{
		UE_LOG(LogTemp, Fatal, TEXT("%s is missing a player controller"), *GetName())
	}
	PlayerController->bShowMouseCursor = true;

	//Initialise CastWidget
	CastWidgetStartingRotation = CastWidget->GetComponentRotation();
	SetCastWidgetLength();

	//Initialise ReelWidget
	if (!ReelWidget) //Ensuring player controller exists.
	{
		UE_LOG(LogTemp, Fatal, TEXT("%s is missing ReelWidget"), *GetName())
	}

	ReelWidget->SetVisibility(ESlateVisibility::Hidden);
}

void AFisher::SetCastWidgetLength()
{
	//Normalising CastWidget scale.
	FVector NormalisedScale = CastWidget->GetComponentScale() / CastWidget->Bounds.BoxExtent.X;
	//Denominator changes depending on which part of the bounds is facing forward.
	CastWidget->SetWorldScale3D(NormalisedScale * CastMaxLength); //World scale in case the parent object gets scaled.
}

void AFisher::SetCastWidgetRotation()
{
	FHitResult HitResult;
	PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, HitResult);
	if (HitResult.bBlockingHit)
	{
		float DeltaX = HitResult.Location.X - CastWidget->GetComponentLocation().X;
		float DeltaY = HitResult.Location.Y - CastWidget->GetComponentLocation().Y;

		MouseDirection = FVector2D(DeltaX, DeltaY); //Direction from actor to mouse.
		MouseDirection.Normalize();

		FRotator NewRotation = CastWidgetStartingRotation;

		NewRotation.Yaw = UKismetMathLibrary::DegAtan2(MouseDirection.Y, MouseDirection.X);
		//Basic trig to get angle from actor to mouse.
		CastWidget->SetWorldRotation(NewRotation);
	}
}

void AFisher::SetCastStrength(float DeltaTime)
{
	CastTime += DeltaTime;
	CastStrength = CastTime / CastDuration; //CastTime/CastDuration to get a number between 0 and 1.

	//(t*t)*(3-(2*t)) creates a curve with a slow start and slow end between values 0 to 1.
	//For more info see https://gamedevbeginner.com/the-right-way-to-lerp-in-unity-with-examples/
	CastStrength = (CastStrength * CastStrength) * (3 - (2 * CastStrength)) * 2;
	//x2 so a value of 1 has a linear gradient so it doesn't slow down.
	if (CastStrength > 1) //Count down. So 1 to 0, instead of 0 to 1.
	{
		CastStrength = CastStrength - 1; //E.G. 1.1 - 1 = 0.1
		CastStrength = 1 - CastStrength;
		// E.G 1 - 0.1 = 0.9. So have gone from 1.1 to 0.9. Counting down instead of up.
	}

	if (CastTime >= CastDuration) { CastTime = 0; }
}

void AFisher::SetCastPoint()
{
	float MouseDirectionMultiplier = CastMaxLength * CastStrength * 2; //Why x2?
	CastPoint = GetActorLocation() + FVector(MouseDirection * MouseDirectionMultiplier, 0.f);
}

void AFisher::OnFailure()
{
	if (AttachedFish)
	{
		AttachedFish = nullptr;
	}
	CastWidget->SetVisibility(true);
	FisherState = EFisherState::Idle;
	CastStrength = 0;
	CastTime = 0;
	ReelWidget->SetVisibility(ESlateVisibility::Hidden);
	ReelLives = 4;//May want avoid hard coding this in case I want to increase this through gameplay items like rods.
}

void AFisher::OnSuccess()
{
	//Do level stuff. Also Reset.
}

void AFisher::DecreaseReelLives()
{
	ReelLives -= 1;
	ReelWidgetPosition = 0;
}

void AFisher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PawnDeltaTime = DeltaTime;
	SetCastWidgetRotation();

	switch (FisherState)
	{
	case EFisherState::Casting:
		SetCastStrength(DeltaTime);
		break;
	case EFisherState::Reeling:
		ReelWidgetTime += DeltaTime;
		if (ReelWidgetTime >= ReelWidgetUpdateDuration)
		{
			SetReelingWidgetPosition();
			ReelWidgetTime = 0.f;
		}

		if (ReelWidgetPosition >= 0.98)
		{
			DecreaseReelLives();
		}
		
		if (ReelLives <= 0)
		{
			OnFailure();
		}
		break;
	default: //Idle
		break;
	}
}

EFisherState AFisher::GetPlayerState()
{
		return FisherState;
}

void AFisher::CastReel()
{
	switch (FisherState)
	{
	case EFisherState::Casting:
		SetCastPoint();
		CastWidget->SetVisibility(false);
		FisherState = EFisherState::Casted;
		break;
	case EFisherState::Casted:
		if (AttachedFish)
		{
			ReelWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
			FisherState = EFisherState::Reeling;
		}
		else
		{
			OnFailure();
		}
		break;
	case EFisherState::Reeling:
		if (UKismetMathLibrary::Abs(ReelWidgetPosition) < 0.25)
		{
			AttachedFish->Pull();
		}
		else if (UKismetMathLibrary::Abs(ReelWidgetPosition) > 0.8)
		{
			DecreaseReelLives();
		}
		break;
	default: //Idle
		FisherState = EFisherState::Casting;
		break;
	}
}

void AFisher::SetReelingWidgetPosition()
{
	//Assumes X is the world forward. Kinda hard codey.
	FVector FishVelocity = AttachedFish->GetVelocity();
	FishVelocity.Normalize();
	
	float Horizontal = FishVelocity.Y;

	//Want to be between 0.3 and 0.8 on both sides of 0.
	float ClampedPostiveHorizontal =  FMath::Clamp(UKismetMathLibrary::Abs(Horizontal), 0.3f, 0.8f);
	Horizontal = (Horizontal > 0) ? ClampedPostiveHorizontal : -ClampedPostiveHorizontal;
	
	ReelWidgetPosition += Horizontal;
	ReelWidgetPosition = FMath::Clamp(ReelWidgetPosition, -1.f, 1.f);
}

void AFisher::AddReelWidgetPosition(float Value)
{
	if (FisherState != EFisherState::Reeling) { return; }
	
	//Only uses value to check if pos or neg. Feels kinda meh.
	if (Value > 0)
	{
		ReelWidgetPosition += ReelWidgetInputSensitivity * PawnDeltaTime;
	}
	else if (Value < 0)
	{
		ReelWidgetPosition -= ReelWidgetInputSensitivity * PawnDeltaTime;
	}
}

FVector AFisher::GetCastPoint() const
{
	return CastPoint;
}

void AFisher::SetAttachedFish(AFish* BaitedFish)
{
	AttachedFish = BaitedFish;
}

// Called to bind functionality to input
void AFisher::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Cast/Reel", IE_Pressed, this, &AFisher::CastReel);
	PlayerInputComponent->BindAxis("ReelDirection",this, &AFisher::AddReelWidgetPosition);
}
