// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"

#include "NativeGameplayTags.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/GameOverChecker/C_GameOverChecker.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/C_ZombieManager.h"
#include "Utility/C_Util.h"
#include "Utility/C_UtilActor.h"

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

#if WITH_EDITOR
	m_UtilActor = InWorld.SpawnActor<AC_UtilActor>();
#endif
	
}

void UC_GameLevelManager::Deinitialize()
{
	m_Players.Empty();
	m_LocalPlayer = nullptr;
	
	Super::Deinitialize();
}

void UC_GameLevelManager::AddPlayer(AC_BasicPlayer* _Player)
{
	m_Players.Add(_Player);
	if (_Player->IsLocallyControlled()) m_LocalPlayer = _Player;
}

void UC_GameLevelManager::RemovePlayer(AC_BasicPlayer* _Player)
{
	if (!_Player) return;
	
	m_Players.Remove(_Player);
	
	if (m_LocalPlayer == _Player)
		m_LocalPlayer = nullptr;
}

bool UC_GameLevelManager::HasAllPlayerDead() const
{
	for (AC_BasicPlayer* Player : m_Players)
		if (Player->GetPlayerMainState() != EPlayerMainState::Dead) return false;
	return true;
}
