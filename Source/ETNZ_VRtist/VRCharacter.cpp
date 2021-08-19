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
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	TeleportPath = CreateDefaultSubobject<USplineComponent>(TEXT("TeleportPath"));
	TeleportPath->SetupAttachment(GetRootComponent());

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	DestinationMarker->SetVisibility(false);

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
	/*
	if (BlinkerMaterialBase != nullptr)
	{
		BlinkerMaterialInstance = UMaterialInstanceDynamic::Create(BlinkerMaterialBase, this);
		PostProcessComponent->AddOrUpdateBlendable(BlinkerMaterialInstance);


	}
	*/
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.Z = 0;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);

	UpdateDestinationMarker();
	TeleportAction(DeltaTime);
	//UpdateBlinkers();
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction(TEXT("GripLeft"), IE_Pressed, this, &AVRCharacter::GripLeft);
	PlayerInputComponent->BindAction(TEXT("GripLeft"), IE_Released, this, &AVRCharacter::ReleaseLeft);
	PlayerInputComponent->BindAction(TEXT("GripRight"), IE_Pressed, this, &AVRCharacter::GripRight);
	PlayerInputComponent->BindAction(TEXT("GripRight"), IE_Released, this, &AVRCharacter::ReleaseRight);

	PlayerInputComponent->BindAction(TEXT("TriggerL"), IE_Pressed, this, &AVRCharacter::TriggerLeft);
	PlayerInputComponent->BindAction(TEXT("TriggerR"), IE_Pressed, this, &AVRCharacter::TriggerRight);
	PlayerInputComponent->BindAction(TEXT("TriggerL"), IE_Released, this, &AVRCharacter::TriggerReleaseLeft);
	PlayerInputComponent->BindAction(TEXT("TriggerR"), IE_Released, this, &AVRCharacter::TriggerReleaseRight);

}

void AVRCharacter::Grip()
{
}

void AVRCharacter::Release()
{
}

void AVRCharacter::TeleportAction_Implementation(float DeltaTime)
{
	if (isTeleporting)
	{
		FVector currentPosition = this->GetActorLocation();
		if (!canIStop(currentPosition, teleportingPosition))
		{
			this->SetActorLocation(FMath::VInterpTo(currentPosition, teleportingPosition, DeltaTime, 15.0));
			this->SetActorEnableCollision(false);
		}
		else
		{
			isTeleporting = false;
			this->SetActorEnableCollision(true);
		}
	}
}

void AVRCharacter::UpdateDestinationMarker()
{
	TArray<FVector> Path;

	FVector Start = RightHand->GetActorLocation();
	FVector End = RightHand->GetActorForwardVector();

	/*
	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
	*/
	FPredictProjectilePathParams Params(
		TeleportProjectileRadius,
		Start,
		End * TeleportProjectileSpeed,
		TeleportSimulationTime,
		ECollisionChannel::ECC_Visibility,
		this
	);
	//Params.DrawDebugType = EDrawDebugTrace::ForOneFrame;
	FPredictProjectilePathResult Result;



	bool bHit = UGameplayStatics::PredictProjectilePath(this, Params, Result);

	if (bHit && !isTeleporting)
	{

		DestinationMarker->SetVisibility(true);
		DestinationMarker->SetWorldLocation(Result.HitResult.Location);

		for (FPredictProjectilePathPointData PointData : Result.PathData)
		{
			Path.Add(PointData.Location);
		}
		DrawTeleportPath(Path);

	}
	else
	{
		TArray<FVector> EmptyPath;
		DrawTeleportPath(EmptyPath);
		DestinationMarker->SetVisibility(false);
	}
}

void AVRCharacter::UpdateSpline(const TArray<FVector>& Path)
{
	TeleportPath->ClearSplinePoints(false);
	for (int32 i = 0; i < Path.Num(); i++)
	{
		FVector LocalPosition = TeleportPath->GetComponentTransform().InverseTransformPosition(Path[i]);
		FSplinePoint Point(i, LocalPosition, ESplinePointType::Curve);
		TeleportPath->AddPoint(Point, false);
	}

	TeleportPath->UpdateSpline();
}

void AVRCharacter::DrawTeleportPath(const TArray<FVector>& Path)
{
	UpdateSpline(Path);

	for (USplineMeshComponent* DynamicMesh : TPPathMeshPool)
	{
		DynamicMesh->SetVisibility(false);
	}
	int32 SegmentNum = Path.Num() - 1;
	for (int32 i = 0; i < SegmentNum; i++)
	{
		if (TPPathMeshPool.Num() <= i)
		{
			USplineMeshComponent* DynamicMesh = NewObject<USplineMeshComponent>(this);
			DynamicMesh->SetMobility(EComponentMobility::Movable);
			DynamicMesh->AttachToComponent(TeleportPath, FAttachmentTransformRules::KeepRelativeTransform);
			DynamicMesh->RegisterComponent();
			DynamicMesh->SetStaticMesh(TeleportArchMesh);
			DynamicMesh->SetMaterial(0, TeleportArchMaterial);
			TPPathMeshPool.Add(DynamicMesh);
		}
		USplineMeshComponent* DynamicMesh = TPPathMeshPool[i];
		DynamicMesh->SetVisibility(true);
		FVector StartPos, StartTangent, EndPos, EndTangent;
		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i, StartPos, StartTangent);
		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i + 1, EndPos, EndTangent);
		DynamicMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
	}
}

/*
void AVRCharacter::UpdateBlinkers()
{

	if (RadiusVsVelocity == nullptr) return;
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Some debug message!"));
	float Speed = GetVelocity().Size();
	float Radius = RadiusVsVelocity->GetFloatValue(Speed);

	BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Radius"), Radius);
}
*/
bool AVRCharacter::canIStop(FVector currentPosition, FVector target)
{
	bool xComp = false;
	bool yComp = false;
	bool zComp = false;

	if (fabs(currentPosition.X - target.X) < 15) { xComp = true; }
	if (fabs(currentPosition.Y - target.Y) < 15) { yComp = true; }
	if (xComp && yComp)
	{
		return true;
	}
	else
	{
		return false;
	}

}

void AVRCharacter::BeginTeleport()
{
	SetActorLocation(DestinationMarker->GetComponentLocation());
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