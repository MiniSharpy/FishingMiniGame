// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AIController.h"
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
	void Escape(float DeltaTime);
	void Move();

	//Rotates the actor to look in the direction of acceleration.
	void LookWhereMoving();
	bool IsCastPointInRange() const;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* FishMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Casting")
	//Max distance in cm for bait to be in range.
	float Range = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Casting")
	//Not exactly FOV. In range 0-1.
	float FOV = 0.8f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Casting")
	//The time in which to wait before changing escape goal.
	float EscapeGoalMaxTime = 5;
	//The elapsed time before changing escape goal. Want the first run to be instant.
	float EscapeGoalTime;
	
	AAIController* AIController;
	AFisher* Fisher;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Pulls the fish towards the fisher. Called in Fisher.cpp
    void Pull();
};
