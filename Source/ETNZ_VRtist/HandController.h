// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MotionControllerComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "HandController.generated.h"

UCLASS()
class ETNZ_VRTIST_API AHandController : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHandController();

	void SetHand(EControllerHand Hand) { MotionController->SetTrackingSource(Hand); }
	void PairController(AHandController* Controller);

	void Grip();
	void Release();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UWidgetInteractionComponent* Widgetinteraction;

private:
	//config
	
	//
	UPROPERTY(VisibleAnywhere)
		UMotionControllerComponent* MotionController;

	

	UPROPERTY(EditDefaultsOnly)
		class UHapticFeedbackEffect_Base* HapticEffect;

	UFUNCTION()
		void ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
	UFUNCTION()
		void ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);
	

	bool CanClimb() const;


	bool bCanClimb = false;
	bool bIsClimbing = false;
	FVector ClimbingStartLocation;

	AHandController* OtherController;
};
