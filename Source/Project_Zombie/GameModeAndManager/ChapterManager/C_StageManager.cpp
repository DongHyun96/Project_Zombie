// Fill out your copyright notice in the Description page of Project Settings.


#include "C_StageManager.h"

#include "GameModeAndManager/C_UIManager.h"

void UC_StageManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("StageManager::Initialize"));
	
	m_GameStageInfosData = LoadObject<UDataTable>(nullptr, TEXT("/Game/DongHyun/DataTable/DT_GameStageInfos.DT_GameStageInfos"));
	
	if (!m_GameStageInfosData)
	{
		UE_LOG(LogTemp, Log, TEXT("[StageManager::Initialize] : Failed to load GameStageInfo DataTable"));
		return;
	}

	TArray<const FGameStageInfo*> StageInfos{};
	m_GameStageInfosData->GetAllRows<const FGameStageInfo>(TEXT("Load Stage infos"), StageInfos);

	for (const FGameStageInfo* StageInfo : StageInfos)
		m_ClearedGameStages.Add({ StageInfo, false });
}

void UC_StageManager::Deinitialize()
{
	Super::Deinitialize();
	
	UE_LOG(LogTemp, Log, TEXT("StageManager::DeInitialize"));
}

const FGameStageInfo* UC_StageManager::GetCurStageInfo() const
{
	if (!m_ClearedGameStages.IsValidIndex(m_CurStageIdx)) return nullptr;
	return m_ClearedGameStages[m_CurStageIdx].Key;	
}

void UC_StageManager::SetCurStageToNextStage()
{
	if (!m_ClearedGameStages.IsValidIndex(m_CurStageIdx))
	{
		PRINT_LOCAL(GetWorld(), "[UC_StageManager::SetCurStageToNextStage] : Invalid Current Stage Index", FColor::Red, 10.f);
		return;
	}
	
	// 현재 Stage Clear처리 및 다음 Stage로 넘어가기
	m_ClearedGameStages[m_CurStageIdx++].Value = true;
}
