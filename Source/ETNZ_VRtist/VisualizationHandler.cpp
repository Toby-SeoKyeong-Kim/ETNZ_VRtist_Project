// Fill out your copyright notice in the Description page of Project Settings.


#include "VisualizationHandler.h"
#include "Math/UnrealMathUtility.h"

TArray<FVector2D> UVisualizationHandler::GetVectorFromCPP(const TArray<float>& InFloatArray, int32 GlobalPtr)
{
	TArray<FVector2D> OutVector;
	int32 ArrayNum = InFloatArray.Num();

	for (size_t i = 0; i < 1024; i++)
	{
		float Yval = 540;
		if (i + GlobalPtr < ArrayNum) {
			Yval = FMath::GetMappedRangeValueClamped(FVector2D(-1.f, 1.f), FVector2D(1040.f, 40.f), InFloatArray[i + GlobalPtr]);
		}
		OutVector.Add(FVector2D((float)(1920.f / 1024.f * i), Yval));
	}
	if (GlobalPtr >= ArrayNum) {
		if (OnDJFinished.IsBound())
		{
			OnDJFinished.Broadcast(true);
		}
	}

	return OutVector;
}

UVisualizationHandler* UVisualizationHandler::CreateVisualization()
{
	UVisualizationHandler* VisualizeManager = NewObject<UVisualizationHandler>();

	return VisualizeManager;
}