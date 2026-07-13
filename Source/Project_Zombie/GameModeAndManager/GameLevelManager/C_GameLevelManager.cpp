// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"

#include "GameModeAndManager/C_ZombieManager.h"
#include "Utility/C_Util.h"

UC_GameLevelManager::UC_GameLevelManager()
{
	static ConstructorHelpers::FClassFinder<UC_ZombieManager> ZMFinder(TEXT("/Game/DongHyun/Manager/BP_ZombieManager"));
	if (ZMFinder.Succeeded()) m_ZombieManagerClass = ZMFinder.Class;
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

	// m_ZombieManagerClass가 제대로 초기화되어 있으면 해당 BP 클래스로, 아니면 C++ 기본 클래스로 생성
	if (m_ZombieManagerClass)
		m_ZombieManager = NewObject<UC_ZombieManager>(this, m_ZombieManagerClass);
	else
	{
		UC_Util::Print("From UC_GameLevelManager::OnWorldBeginPlay : ZombieManagerClass missing. Using default c++ class ", FColor::Red, 10.f);
		m_ZombieManager = NewObject<UC_ZombieManager>(this);
	}

	if (m_ZombieManager) m_ZombieManager->OnWorldBeginPlay();
}
