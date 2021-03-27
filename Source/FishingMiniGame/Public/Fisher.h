// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"
#include "Fisher.generated.h"

class AFish;
class USpringArmComponent;
class UWidgetComponent;
class APlayerController;

UENUM()
enum EFisherState
{
	Idle,
	Casting,
	Casted,
	Reeling
};

UCLASS()
class FISHINGMINIGAME_API AFisher : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AFisher();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void OnFailure();
	UFUNCTION(BlueprintCallable)
	void OnSuccess();//Called in level blueprint.

	EFisherState FisherState;

	//Player Controller related.
	APlayerController* PlayerController;
	FVector2D MouseDirection;//This is needed to get the location of the cast.

	//The fish handles attaching itself.
	AFish* AttachedFish;

	float PawnDeltaTime;
	
#pragma region Casting
	UFUNCTION(CallInEditor)
	void SetCastWidgetLength();
	//Sets the world rotation of the cast widget so that it points at the mouse position.
	void SetCastWidgetRotation();
	void SetCastStrength(float DeltaTime);
	void SetCastPoint();
	void CastReel(); //Input action.

	//The widget updates certain visual aspects itself.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* CastWidget;
	//Some shenanigans(quaternions?) mean when setting rotation not just yaw has to be set.
	FRotator CastWidgetStartingRotation;
	//The position where the bait is.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Casting")
	FVector CastPoint;
	//Max distance in cm the bait can reach. Also the length of the widget.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Casting")
	float CastMaxLength = 400.f;
	//This is required for the directional indicator blueprint code to update its progress bar. Between 0 and 1.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Casting")
	float CastStrength; 
	//The time it takes for the cast strength to go to full, and back to zero (0 to 1 to 0). Used to generate CastStrength.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Casting")
	float CastDuration = 2.5f;
	//Used in the algorithm to generate CastStrength.
	float CastTime;
#pragma endregion Casting

#pragma region Reeling
	void SetReelingWidgetPosition();
	void AddReelWidgetPosition(float Value);
	void DecreaseReelLives();

	//This is only needed to hide the widget depending on player state. Added to viewport in BP.
	UPROPERTY(BlueprintReadWrite, Category = "Components")
	UUserWidget* ReelWidget;
	//ReelingWidget updates the marker's position itself using ReelingWidgetPosition in blueprint.
	//Between -1 and 1. -0.25 to 0.25 is the safe zone. 0.8 to 1 and -0.8 to -1 are loss zones.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Reeling") 
	float ReelWidgetPosition;
	//This is to add the ReelingWidgetPosition every x seconds.
	float ReelWidgetTime;		
	UPROPERTY(EditDefaultsOnly, Category="Reeling")
	//How many seconds to wait before getting new values for the reel widget.
	float ReelWidgetUpdateDuration = 2.5f;//Might want to randomise each time?
	//The speed at which the reel widget indicator will go from one side to the other.
	//A value of 2 will go from one of the entire widget to the other in a second. 
	UPROPERTY(EditDefaultsOnly, Category="Reeling")
	float ReelWidgetInputSensitivity = 4.f;
	//How many chances the fisher gets if they attempt to pull when in the red zone, or if they max out ReelWidgetPosition.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reeling")
	int ReelLives = 4;
#pragma endregion Reeling

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	EFisherState GetFisherState();
	FVector GetCastPoint() const;
	void SetAttachedFish(AFish* BaitedFish);
	AFish* GetAttachedFish();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
