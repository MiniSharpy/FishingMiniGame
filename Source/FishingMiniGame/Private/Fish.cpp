// Fill out your copyright notice in the Description page of Project Settings.


#include "Fish.h"
#include "Fisher.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AFish::AFish()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Fisher"));
	
	FishMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Fish"));
	FishMesh->SetupAttachment(RootComponent);
	FishMesh->SetCanEverAffectNavigation(false);
}

// Called when the game starts or when spawned
void AFish::BeginPlay()
{
	Super::BeginPlay();
	Fisher = Cast<AFisher>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));

	AIController = Cast<AAIController>(GetController());
	if (!AIController) //Ensuring AIController exists.
	{
		UE_LOG(LogTemp, Fatal, TEXT("%s is missing an AI controller"), *GetName())
	}

	EscapeGoalTime = EscapeGoalMaxTime;
}

void AFish::Escape(float DeltaTime)
{
	EscapeGoalTime += DeltaTime;
	if (EscapeGoalTime < EscapeGoalMaxTime && !GetVelocity().IsNearlyZero()) { return; }//Get new point if not moving.
	
	FNavLocation Goal;
	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	NavigationSystem->GetRandomPointInNavigableRadius(Fisher->GetCastPoint(), 1000, Goal);
	AIController->MoveToLocation(Goal.Location);
	EscapeGoalTime = 0.f;
}

void AFish::Move()
{
	//Move randomly in navigable area.
}

void AFish::Pull()
{
	//Getting actor locations
	FVector FisherLocation = Fisher->GetActorLocation();
	FisherLocation.Z = 0.f;
	FVector FishLocation = GetActorLocation();
	FishLocation.Z = 0.f;
	//Get direction from fisher to fish.
	FVector FisherToFishDirection = FisherLocation - FishLocation;
	FisherToFishDirection.Normalize();

	//Pull towards the fisher.
	FVector NewFishLocation = GetActorLocation() + FisherToFishDirection * 25;
	SetActorLocation(NewFishLocation);
}

void AFish::LookWhereMoving()
{
	//Don't want to alter rotation if velocity is zero. This is because it will set the yaw to 0, which is unwanted
	//because we want the actor to not suddenly point in the same direction evey time it's motionless.
	if (GetVelocity().IsNearlyZero()) { return; }
	
	//Getting acceleration.
	FVector Acceleration = GetVelocity();
	
	Acceleration.Normalize();

	//Getting LookDirection. Only want yaw to prevent unexpected results.
	FRotator LookRotation = Acceleration.Rotation();
	LookRotation.Roll = 0; LookRotation.Pitch = 0;
	SetActorRotation(LookRotation);
}

bool AFish::IsCastPointInRange() const
{
	//Need to ensure that all calculations are on the same level. Might be best to switch to FVector2D?
	FVector SameZActorLocation = GetActorLocation();
	SameZActorLocation.Z = Fisher->GetCastPoint().Z;

	FVector BaitDirection = Fisher->GetCastPoint() - SameZActorLocation;
	BaitDirection.Normalize();
	float DirectionDotProduct = FVector::DotProduct(GetActorForwardVector(), BaitDirection);//Maybe for designer ease convert to angle?
	float Distance = FVector::Distance(Fisher->GetCastPoint(), SameZActorLocation);

	if (Distance <= Range && DirectionDotProduct >= FOV)
	{
		return true;
	}
	
	return false;
}

void AFish::OnFailure()
{
	Fisher->OnFailure();
}

// Called every frame
void AFish::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LookWhereMoving();
	
	if (Fisher->GetFisherState() == EFisherState::Casted && IsCastPointInRange())
	{
		//Kill movement.
		Fisher->SetAttachedFish(this);
		EscapeTime += DeltaTime;
		if (EscapeTime >= MaxEscapeTime)
		{
			OnFailure();
		}
	}
	else if (Fisher->GetAttachedFish() == this && Fisher->GetFisherState() == EFisherState::Reeling)//Originally had a bool, IsAttached, but this means it would manually need to be reset. Also need to wait for user to click before trying to escape.
	{
		EscapeTime = 0.f;
		Escape(DeltaTime);
	}
	else
	{
		Move();
	}
}
