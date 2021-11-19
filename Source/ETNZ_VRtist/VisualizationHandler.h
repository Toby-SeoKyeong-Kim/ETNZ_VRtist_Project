// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/Vector2D.h"
#include "UObject/NoExportTypes.h"
#include "VisualizationHandler.generated.h"



/**
 * Delegate broadcast to get the Finish bool
 *
 * @param IsFinished Whether the track playing is finished
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioFinished, bool, IsFinished);
/**
 * 
 */
UCLASS(BlueprintType, Category = "Visualization")
class ETNZ_VRTIST_API UVisualizationHandler : public UObject
{
	GENERATED_BODY()

		bool IsNameStableForNetworking() const override {
		return true;
	}

	bool IsSupportedForNetworking() const override
	{
		return true;
	}
	UPROPERTY(BlueprintAssignable, Category = "Visualization")
		FOnAudioFinished OnDJFinished;

	UFUNCTION(BlueprintCallable, Category = "DJMachine")
		TArray<FVector2D> GetVectorFromCPP(const TArray<float>& InFloatArray, int32 GlobalPtr);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "Create, Visualization"), Category = "Visualization")
		static UVisualizationHandler* CreateVisualization();
};
