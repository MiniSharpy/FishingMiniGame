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

// Called every frame
void AFish::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Fisher->GetFisherState() == EFisherState::Casted && IsCastPointInRange())
	{
		//Kill movement. Break after X time and call OnFailure() in fisher?
		Fisher->SetAttachedFish(this);
	}

	if (Fisher->GetAttachedFish() == this && Fisher->GetFisherState() == EFisherState::Reeling)//Originally had a bool, IsAttached, but this means it would manually need to be reset. Also need to wait for user to click before trying to escape.
	{
		Escape();//Try to stick around bait point. Tugs on ReelWidget will be determined by direction.
	}
	else
	{
		Move();
	}
}
