// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ChapterManager.h"

void UC_ChapterManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("ChapterManager::Initialize"));
}

void UC_ChapterManager::Deinitialize()
{
	Super::Deinitialize();
	
	UE_LOG(LogTemp, Log, TEXT("ChapterManager::DeInitialize"));
}
