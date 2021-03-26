// Fill out your copyright notice in the Description page of Project Settings.


#include "Fish.h"
#include "Fisher.h"
#include "Kismet/GameplayStatics.h"

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
}

bool AFish::IsCastPointInRange() const
{
	FVector BaitDirection = Fisher->GetCastPoint() - GetActorLocation();
	BaitDirection.Normalize();
	float DirectionDotProduct = FVector::DotProduct(GetActorForwardVector(), BaitDirection);//Maybe for ease convert to angle?

	float Distance = FVector::Distance(Fisher->GetCastPoint(), GetActorLocation());

	if (Distance <= Range && DirectionDotProduct >= FOV)
	{
		return true;
	}
	
	return false;
}

// Called every frame
void AFish::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsCastPointInRange() && Fisher->GetPlayerState() == EFisherState::Casted)
	{
		//Kill movement. Break after X time and set fail for player?
		Fisher->SetAttachedFish(this);
	}

	if (Fisher->GetPlayerState() ==  EFisherState::Reeling)
	{
		Escape();//Try to stick around bait point. Tugs on ReelWidget will be determined by direction.
	}
	else
	{
		Move();
	}
}
