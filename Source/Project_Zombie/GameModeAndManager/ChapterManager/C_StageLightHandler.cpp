// Fill out your copyright notice in the Description page of Project Settings.


#include "C_StageLightHandler.h"

#include "C_StageManager.h"
#include "EngineUtils.h"
#include "GlobalEnum.h"
#include "Components/LightComponent.h"
#include "Engine/DirectionalLight.h"
#include "GameModeAndManager/C_UIManager.h"


AC_StageLightHandler::AC_StageLightHandler()
{
	PrimaryActorTick.bCanEverTick = false;

	SetReplicates(true);
	bAlwaysRelevant = true;
}

void AC_StageLightHandler::BeginPlay()
{
	Super::BeginPlay();

	// Lighting 관련된 Actor 모두 찾기
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor) continue;

		if (ADirectionalLight* DirectionalLight = Cast<ADirectionalLight>(Actor))
		{
			if (DirectionalLight->ActorHasTag(TEXT("DL0")))
				m_DirectionalLights.Add(EStageTimeOfDay::Day, DirectionalLight);	
			else if (DirectionalLight->ActorHasTag(TEXT("DL1")))
				m_DirectionalLights.Add(EStageTimeOfDay::Night, DirectionalLight);
		}

		if (APostProcessVolume* PostProcessVolume = Cast<APostProcessVolume>(Actor))
		{
			if (PostProcessVolume->ActorHasTag(TEXT("PP0")))
				m_PostProcessVolumes.Add(EStageTimeOfDay::Day, PostProcessVolume);
			else if (PostProcessVolume->ActorHasTag(TEXT("PP1")))
				m_PostProcessVolumes.Add(EStageTimeOfDay::Night, PostProcessVolume);
		}
	}

	const uint8 TimeOfDayCnt = static_cast<uint8>(EStageTimeOfDay::End);
	
	if (m_DirectionalLights.Num() != TimeOfDayCnt || m_PostProcessVolumes.Num() != TimeOfDayCnt)
	{
		PRINT_LOCAL(GetWorld(), "[AC_StageLightHandler::BeginPlay] : Lighting Actor not all found!]", FColor::Red, 10.f);
		return;
	}
	
	PRINT_LOCAL(GetWorld(), "[AC_StageLightHandler::BeginPlay] : Lighting Actor found!]", FColor::Yellow, 10.f);
	const EStageTimeOfDay CurTimeOfDay = GetGameInstance()->GetSubsystem<UC_StageManager>()->GetCurStageInfo()->TimeOfDay;

	for (uint8 i = 0; i < TimeOfDayCnt; ++i)
	{
		EStageTimeOfDay TimeOfDay = static_cast<EStageTimeOfDay>(i);
		if (TimeOfDay == CurTimeOfDay)
		{
			// m_DirectionalLights[TimeOfDay].
			m_DirectionalLights[TimeOfDay]->GetLightComponent()->SetVisibility(true);
			m_PostProcessVolumes[TimeOfDay]->bEnabled = true;
		}
		else
		{
			m_DirectionalLights[TimeOfDay]->GetLightComponent()->SetVisibility(false);
			m_PostProcessVolumes[TimeOfDay]->bEnabled = false;
		}
	}
}
