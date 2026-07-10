// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/C_GameLevelManager.h"

#include "C_GameMode_GameLv.h"
#include "GameFramework/GameModeBase.h"

UC_GameLevelManager::UC_GameLevelManager()
{
}

bool UC_GameLevelManager::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
		return false;
	
	UWorld* World = Cast<UWorld>(Outer);
	if (!World) return false;
	
	const FString MapName = World->GetMapName();
	return MapName.Contains("GameLevel"); 
}

void UC_GameLevelManager::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}
