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
	UFUNCTION(BlueprintCallable, Category = "My Functions")
		void BeginTeleport();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Grip();
	void Release();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* DestinationMarker;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool isTeleporting = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FVector teleportingPosition;
	UPROPERTY(BlueprintReadWrite)
		class UPostProcessComponent* PostProcessComponent;
	UFUNCTION(BlueprintCallable, Category = "HandController")
		void ToggleHidden(bool bNewHidden);
	UFUNCTION(BlueprintCallable, Category = "HandController")
		void IsDJController(bool isController);
	UFUNCTION(BlueprintCallable, Category = "HandController")
		void HandIndex(float LeftIndex, float RightIndex);
	UFUNCTION(BlueprintCallable, Category = "SceneComp")
		void RotateRoot(float inFloat);
private:
	UFUNCTION()
		void UpdateDestinationMarker();
	UFUNCTION()
		void UpdateSpline(const TArray<FVector>& Path);
	UFUNCTION()
		void DrawTeleportPath(const TArray<FVector>& Path);
	UFUNCTION(Server, Reliable)
		virtual void TeleportAction(float DeltaTime);
	UPROPERTY(EditAnywhere)
		float MaxTeleportDistance = 1000;
	UPROPERTY(EditAnywhere)
		float TeleportProjectileRadius = 10;
	UPROPERTY(EditAnywhere)
		float TeleportProjectileSpeed = 1000;
	UPROPERTY(EditAnywhere)
		float TeleportSimulationTime = 10;

	void GripLeft() { LeftHand->Grip(); };
	void ReleaseLeft() { LeftHand->Release(); };
	void GripRight() { RightHand->Grip(); };
	void ReleaseRight() { RightHand->Release(); };
	void TriggerLeft() {  LeftHand->Widgetinteraction->PressPointerKey(FKey(EKeys::LeftMouseButton)); };
	void TriggerRight() { RightHand->Widgetinteraction->PressPointerKey(FKey(EKeys::LeftMouseButton)); };
	void TriggerReleaseLeft() { LeftHand->Widgetinteraction->ReleasePointerKey(FKey(EKeys::LeftMouseButton)); };
	void TriggerReleaseRight() { RightHand->Widgetinteraction->ReleasePointerKey(FKey(EKeys::LeftMouseButton)); };
	bool canIStop(FVector currentPosition, FVector target);
	bool IsController = false;
private:

	UPROPERTY(VisibleAnywhere)
		class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* VRRoot;

	UPROPERTY()
		class UMaterialInstanceDynamic* BlinkerMaterialInstance;

	UPROPERTY()
		class AHandController* LeftHand;

	UPROPERTY()
		class AHandController* RightHand;

	UPROPERTY()
		TArray<class USplineMeshComponent*> TPPathMeshPool;
	UPROPERTY(EditDefaultsOnly)
		class UStaticMesh* TeleportArchMesh;
	UPROPERTY(EditDefaultsOnly)
		class UMaterialInterface* TeleportArchMaterial;

	UPROPERTY(VisibleAnywhere)
		class USplineComponent* TeleportPath;

	UPROPERTY(EditAnywhere)
		class UMaterialInterface* BlinkerMaterialBase;

	UPROPERTY(EditAnywhere)
		class UCurveFloat* RadiusVsVelocity;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AHandController> HandControllerClass;
};
