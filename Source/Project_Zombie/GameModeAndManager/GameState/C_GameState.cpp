// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GameState.h"

#include "Actor/Character/Player/C_BasicPlayer.h"

#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/ChapterManager/C_StageManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"

#include "Net/UnrealNetwork.h"


void AC_GameState::BeginPlay()
{
	Super::BeginPlay();
	
	// PRINT_LOCAL(GetWorld(), "GameState BeginPlay", FColor::MakeRandomColor(), 10.f);
	// UC_Util::Print("GameState::BeginPlay", CUR_TICK_COLOR, 10.f);

	// if (!LEVEL_MANAGER)
	
	// 나중에 들어오는 친구들은 어떡함? -> 알아서 서버환경의 CurStageIdx로 맞춰지나?
	
	// GetWorld()->GetTimerManager()

	if (HasAuthority())
	{
		// 서버 환경인 경우, CurStageIdx를 수정함으로써 Replicate 처리를 유도하여 클라이언트의 CurStageIdx를 동기화 처리시킨다
		// 이거 근데, 추후에 들어온 Client Player 또한 해당 StageIdx를 부여받을 수 있는지...(아마도 새로운 GameState가 서버환경에서도 만들어짐에 따라
		// Replicate 함수 호출될 수 있을 듯?)
		// 더 명확하게는 클라 환경에서는 OnRep_CurStageIdx가 호출이 되어야 비로소 StageManager의 CurStageIdx가 알맞은 값으로 동기화됨 
		m_CurStageIdx = GetGameInstance()->GetSubsystem<UC_StageManager>()->GetCurStageIdx();
	}
}

void AC_GameState::OnRep_CurStageIdx()
{
	PRINT_LOCAL(GetWorld(), "OnRep_CurStageIdx", FColor::MakeRandomColor(), 10.f);

	// Client 환경에서의 StageManager에서 또한 CurStageIdx 동기화 처리를 해준다
	UC_StageManager* StageManager = GetGameInstance()->GetSubsystem<UC_StageManager>();
	StageManager->SetCurStageIdx(m_CurStageIdx);
}

void AC_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AC_GameState, m_CurStageIdx);
}

void AC_GameState::SetCurStageIdx(uint8 _StageIdx)
{
	// 오로지 서버에서만 호출할 것

	if (!LEVEL_MANAGER)
	{
		PRINT_LOCAL(GetWorld(), "[AC_GameState::SetCurStageIdx] : Level Manager nullptr", FColor::Red, 10.f);
		return;
	}
	
	if (!LEVEL_MANAGER->GetLocalPlayer())
	{
		PRINT_LOCAL(GetWorld(), "[AC_GameState::SetCurStageIdx] : LocalPlayer nullptr", FColor::Red, 10.f);
		return;
	}
	
	if (!LEVEL_MANAGER->GetLocalPlayer()->HasAuthority())
	{
		PRINT_LOCAL(GetWorld(), "[AC_GameState::SetCurStageIdx] : Do not call SetCurStageIdx from client env!", FColor::Red, 10.f);
		return;
	}
	
	m_CurStageIdx = _StageIdx;
}
