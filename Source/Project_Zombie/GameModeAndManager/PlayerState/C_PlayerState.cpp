// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerState.h"

#include "Net/UnrealNetwork.h"

AC_PlayerState::AC_PlayerState()
{
}

void AC_PlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AC_PlayerState, m_bIsHost);
}

void AC_PlayerState::CopyProperties(APlayerState* NewPlayerState)
{
	Super::CopyProperties(NewPlayerState);

	if (NewPlayerState)
	{
		AC_PlayerState* NewPS = Cast<AC_PlayerState>(NewPlayerState);
		if (NewPS)
		{
			// 1. 호스트 여부 이사
			NewPS->m_bIsHost = this->m_bIsHost;

			// 2. 인벤토리 항목 리스트(TArray) 이사
			NewPS->SavedInventoryContainers = this->SavedInventoryContainers;

			// 3. 스탯 및 단계 TMap 이사
			NewPS->SavedStats = this->SavedStats;
			NewPS->SavedStatGrades = this->SavedStatGrades;

			UE_LOG(LogTemp, Log, TEXT("[Seamless] PlayerState 데이터(Array/Map) 카피 완료!"));
		}
	}
}
