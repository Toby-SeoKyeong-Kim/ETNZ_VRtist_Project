// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/Vector2D.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HotcueManager.generated.h"


/**
 * 
 */
UCLASS()
class ETNZ_VRTIST_API UHotcueManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		bool IsNameStableForNetworking() const override {
		return true;
		}	

		bool IsSupportedForNetworking() const override
		{
			return true;
		}

		UFUNCTION(BlueprintCallable, Category = "DJMachine", meta = (keywords = "HotCue"))
		static bool SaveHotcueData(FString SaveDir, FString FileName, TArray<FString> SaveText);

		UFUNCTION(BlueprintCallable, Category = "DJMachine", meta = (keywords = "HotCue"))
		static bool LoadHotcueData(TArray< FString >& Result, FString LoadDir);

		
};
