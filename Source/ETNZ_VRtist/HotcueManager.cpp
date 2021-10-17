// Fill out your copyright notice in the Description page of Project Settings.


#include "HotcueManager.h"
#include "Misc/FileHelper.h"
#include "Math/UnrealMathUtility.h"
#include "Hal/PlatformFilemanager.h"



bool UHotcueManager::SaveHotcueData(FString SaveDir, FString FileName, TArray<FString> SaveText)
{
	SaveDir += "//";
	SaveDir += FileName;

	FString FinalString = "";

	for (FString& Each : SaveText) {
		FinalString += Each;
		FinalString += LINE_TERMINATOR;
	}


	return FFileHelper::SaveStringToFile(FinalString, *SaveDir);
}

bool UHotcueManager::LoadHotcueData(TArray<FString>& Result, FString LoadDir)
{
	return FFileHelper::LoadFileToStringArray(Result, *LoadDir);
}


/*
TArray<FVector2D> UHotcueManager::GetVectorFromCPP(const TArray<float>& InFloatArray, int32 GlobalPtr)
{
	TArray<FVector2D> OutVector;
	int32 ArrayNum = InFloatArray.Num();

	for (size_t i = 0; i < 1024; i++)
	{
		float Yval = 540;
		if (i + GlobalPtr < ArrayNum) {
			Yval = FMath::GetMappedRangeValueClamped(FVector2D(-1.f, 1.f), FVector2D(940.f, 140.f), InFloatArray[i + GlobalPtr]);
		}
		OutVector.Add(FVector2D((float)(1920.f/1024.f * i), Yval));
	}
	if (GlobalPtr >= ArrayNum) {
		if (OnResult.IsBound())
		{
			OnResult.Broadcast(true);
		}
	}

	return OutVector;
}

UHotcueManager* UHotcueManager::CreateVisualization()
{
	UHotcueManager* HotcueManager = NewObject<UHotcueManager>();

	return HotcueManager;
}

*/