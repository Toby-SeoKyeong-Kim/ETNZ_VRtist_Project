// Fill out your copyright notice in the Description page of Project Settings.


#include "HotcueManager.h"
#include "Misc/FileHelper.h"
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