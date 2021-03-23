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
enum EPlayerState
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
	UFUNCTION(CallInEditor)
	void SetCastWidgetLength();
	//Sets the world rotation of the cast widget so that it points at the mouse position.
	void SetCastWidgetRotation();
	void SetCastStrength(float DeltaTime);
	void SetCastPoint();
	void OnFailure();
	UFUNCTION(BlueprintCallable)
	void OnSuccess();//Called in level blueprint.
	
	void CastReel(); //Input action.

	
	EPlayerState PlayerState;

	//Player Controller related.
	APlayerController* PlayerController;
	FVector2D MouseDirection;//This is needed to get the location of the cast.

	//The fish handles attaching itself.
	AFish* AttachedFish;
	
#pragma region CastingVariables
	//The widget updates certain visual aspects itself.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* CastWidget;
	//Some shenanigans(quaternions?) mean when setting rotation not just yaw has to be set.
	FRotator CastWidgetStartingRotation;
	//The position where the bait is.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Casting")
	FVector CastPoint;
	//Max distance in cm the bait can reach. Also the length of the widget.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Casting")
	float CastMaxLength = 400.f;
	//This is required for the directional indicator blueprint code to update its progress bar. Between 0 and 1.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Casting")
	float CastStrength; 
	//The time it takes for the cast strength to go to full, and back to zero (0 to 1 to 0). Used to generate CastStrength.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Casting")
	float CastDuration = 2.5f;
	//Used in the algorithm to generate CastStrength.
	float CastTime;
#pragma endregion CastingVariables

#pragma region ReelingVariables
	//This is only needed to hide the widget depending on player state. Added to viewport in BP.
	UPROPERTY(BlueprintReadWrite, Category = "Components")
	UUserWidget* ReelWidget;
	//ReelingWidget updates the marker's position itself using ReelingWidgetPosition in blueprint.
	//Between -1 and 1. -0.25 to 0.25 is the safe zone. 0.8 to 1 and -0.8 to -1 are loss zones.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Casting") 
	float ReelingWidgetPosition;
#pragma endregion ReelingVariables

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FVector GetCastPoint() const;
	void SetAttachedFish(AFish* BaitedFish);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
