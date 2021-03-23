// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Fish.generated.h"

class AFisher;
UCLASS()
class FISHINGMINIGAME_API AFish : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AFish();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* FishMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Casting")
	//Max distance in cm for bait to be in range.
	float Range = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Casting")
	//Not exactly FOV. In range 0-1.
	float FOV = 0.8f;
	
	bool IsCastPointInRange() const;

	AFisher* Fisher;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
