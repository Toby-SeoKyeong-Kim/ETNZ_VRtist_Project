// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MotionControllerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Math/Vector.h"
#include "HandController.h"

// Sets default values
AVRCharacter::AVRCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	//SetRootComponent(VRRoot);

	VRRoot->SetupAttachment(GetRootComponent());
	

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	
}
void AVRCharacter::IsPicker(bool isPicker)
{
	bIsPicker = isPicker;
}
// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	

	if (!bIsPicker) {
		LeftHand = GetWorld()->SpawnActor<AHandController>(HandControllerClass);
		if (LeftHand != nullptr) {
			LeftHand->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
			LeftHand->SetOwner(this);
			LeftHand->SetHand(EControllerHand::Left);
			LeftHand->Widgetinteraction->PointerIndex = 0.0;
			if (IsController)
			{
				LeftHand->Widgetinteraction->PointerIndex = 2.0;
				LeftHand->SetActorHiddenInGame(true);
			}
		}


		RightHand = GetWorld()->SpawnActor<AHandController>(HandControllerClass);
		if (RightHand != nullptr) {
			RightHand->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
			RightHand->SetOwner(this);
			RightHand->SetHand(EControllerHand::Right);
			RightHand->Widgetinteraction->PointerIndex = 1.0;
			if (IsController)
			{
				RightHand->Widgetinteraction->PointerIndex = 3.0;
				RightHand->SetActorHiddenInGame(true);
			}
		}
		LeftHand->PairController(RightHand);
		RightHand->PairController(LeftHand);
	}
	
	
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!IsController && !bIsPicker)
	{
		FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
		NewCameraOffset.Z = 0;
		AddActorWorldOffset(NewCameraOffset);
		VRRoot->AddWorldOffset(-NewCameraOffset);
	}
	/*
	FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.Z = 0;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);
	*/
	
	//UpdateBlinkers();
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	/*
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AVRCharacter::MoveRight);
	*/
	PlayerInputComponent->BindAction(TEXT("TriggerL"), IE_Pressed, this, &AVRCharacter::TriggerLeft);
	PlayerInputComponent->BindAction(TEXT("TriggerR"), IE_Pressed, this, &AVRCharacter::TriggerRight);
	PlayerInputComponent->BindAction(TEXT("TriggerL"), IE_Released, this, &AVRCharacter::TriggerReleaseLeft);
	PlayerInputComponent->BindAction(TEXT("TriggerR"), IE_Released, this, &AVRCharacter::TriggerReleaseRight);
}



void AVRCharacter::ToggleHidden(bool bNewHidden) {
	if (RightHand != nullptr) {
		RightHand->SetActorHiddenInGame(bNewHidden);
		if (IsController) {
			RightHand->Widgetinteraction->bShowDebug = !bNewHidden;
		}
	}
	if (LeftHand != nullptr) {
		LeftHand->SetActorHiddenInGame(bNewHidden);
		if (IsController) {
			LeftHand->Widgetinteraction->bShowDebug = !bNewHidden;
		}
	}
}

void AVRCharacter::IsDJController(bool isController)
{
	
	IsController = isController;

}

void AVRCharacter::HandIndex(float LeftIndex, float RightIndex)
{
	if (RightHand != nullptr) {
		RightHand->Widgetinteraction->PointerIndex = LeftIndex;
	}
	if (LeftHand != nullptr) {
		LeftHand->Widgetinteraction->PointerIndex = RightIndex;
	}
}
void AVRCharacter::RotateRoot(float inFloat)
{
	FRotator EmptyVector;
	EmptyVector.Yaw = inFloat;
	VRRoot->AddLocalRotation(EmptyVector, false, nullptr, ETeleportType::None);
}

void AVRCharacter::MoveForward(float throttle)
{
	AddMovementInput(throttle * Camera->GetForwardVector());
}

void AVRCharacter::MoveRight(float throttle)
{
	AddMovementInput(throttle * Camera->GetRightVector());
}