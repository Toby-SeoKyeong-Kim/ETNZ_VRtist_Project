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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UMotionControllerComponent* MotionController;
	void SetHand(EControllerHand Hand) { MotionController->SetTrackingSource(Hand); HandSide = Hand; }
	void PairController(AHandController* Controller);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UWidgetInteractionComponent* Widgetinteraction;
	UFUNCTION(BlueprintCallable, Category = "HandController")
		EControllerHand GetHandSide();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool IsDrawingCpp = false;
	UFUNCTION(BlueprintPure, Category = "HandController")
		AHandController* GetOtherHand();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool EditingEQHigh1 = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool EditingEQHigh2 = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool EditingEQMid1 = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool EditingEQMid2 = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool EditingEQLow1 = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool EditingEQLow2 = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool EditingFreq1 = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool EditingFreq2 = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool EditingQ1 = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool EditingQ2 = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool EditingCrossFader = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool EditingVol1 = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool EditingVol2 = false;
private:
	//config
	
	//
	

	AHandController* OtherController;
	EControllerHand HandSide;
};
