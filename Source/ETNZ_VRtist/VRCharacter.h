// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "HandController.h"
#include "VRCharacter.generated.h"


UCLASS()
class ETNZ_VRTIST_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool isTeleporting = false;

	UFUNCTION(BlueprintCallable, Category = "HandController")
		void ToggleHidden(bool bNewHidden);
	UFUNCTION(BlueprintCallable, Category = "HandController")
		void IsDJController(bool isController);
	UFUNCTION(BlueprintCallable, Category = "HandController")
		void HandIndex(float LeftIndex, float RightIndex);
	UFUNCTION(BlueprintCallable, Category = "SceneComp")
		void RotateRoot(float inFloat);
	UFUNCTION(BlueprintCallable, Category = "HandController")
		void IsPicker(bool isPicker);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* VRRoot;
private:
	
	void TriggerLeft() { LeftHand->Widgetinteraction->PressPointerKey(FKey(EKeys::LeftMouseButton)); };
	void TriggerRight() { RightHand->Widgetinteraction->PressPointerKey(FKey(EKeys::LeftMouseButton)); };
	void TriggerReleaseLeft() { LeftHand->Widgetinteraction->ReleasePointerKey(FKey(EKeys::LeftMouseButton)); };
	void TriggerReleaseRight() { RightHand->Widgetinteraction->ReleasePointerKey(FKey(EKeys::LeftMouseButton)); };
	bool IsController = false;
	bool bIsPicker = false;
private:

	UPROPERTY(VisibleAnywhere)
		class UCameraComponent* Camera;

	void MoveForward(float throttle);
	void MoveRight(float throttle);

	UPROPERTY()
		class AHandController* LeftHand;

	UPROPERTY()
		class AHandController* RightHand;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AHandController> HandControllerClass;
};
