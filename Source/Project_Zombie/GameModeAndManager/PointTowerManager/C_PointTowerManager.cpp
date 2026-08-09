// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PointTowerManager.h"
#include "../C_ZombieManager.h"
#include "Actor/Character/NPC/Enemy/Zombie/Spawn/C_SpawnArea.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "Actor/GameOverChecker/C_GameOverChecker.h"
#include "Actor/PointTower/C_PointTower.h"
#include "GameModeAndManager/C_GameMode_GameLv.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"

UC_PointTowerManager::UC_PointTowerManager()
{
	
}

void UC_PointTowerManager::OnWorldBeginPlay()
{
	m_CurrentSequenceIndex = 0;
	
	// 거점 활성화까지 여유시간을 줌 (플레이어 기다리기 처리 등)
	GetWorld()->GetTimerManager().SetTimer
	(
		m_FirstPointOpenWaitTimerHandle, this, &UC_PointTowerManager::StartActivateCurrentPointsSequence, 20.f, false
	);
}

bool UC_PointTowerManager::WorldTick(float _DeltaTime)
{
	// 자체 제작 Tick (GameMode의 Tick에서 호출 걸어둠)

	// 게임 시작까지 기다리기 (다른 플레이어 접속 등을 기다리는 이유도 있다)
	// TODO : 움직이지 못하게 처리를 해버릴까 생각 중
	if (!GetWorld()->GetTimerManager().IsTimerActive(m_FirstPointOpenWaitTimerHandle)) return false;
	
	float LeftTime = GetWorld()->GetTimerManager().GetTimerRemaining(m_FirstPointOpenWaitTimerHandle);

	// UI 표시용 올림값 구하기
	const int32 CurrentSecLeftInt = FMath::CeilToInt(LeftTime);
	if (CurrentSecLeftInt != m_GameStartTimeLeftInt)
	{
		if (GAME_LV_GAME_MODE(this) && GAME_LV_GAME_MODE(this)->GetGameOverChecker())
			GAME_LV_GAME_MODE(this)->GetGameOverChecker()->Multicast_UpdateGameStartLeftTime(CurrentSecLeftInt);
		
		m_GameStartTimeLeftInt = CurrentSecLeftInt;
		
		/*if (m_GameStartTimeLeftInt <= 0) // 게임 시작 처리 (는 알아서 Timer에 의해서 시작됨)
		{
			
		}*/
	}
	
	return true;
}

void UC_PointTowerManager::StartActivateCurrentPointsSequence()
{
	if (!m_PointTowers.IsValidIndex(m_CurrentSequenceIndex))
	{
		UC_Util::Print("[UC_PointTowerManager::StartActivateCurrentPointsSequence] : Invalid Current Sequence index received!", FColor::Red, 10.f);
		return;
	}

	for (AC_PointTower* PointTower : m_PointTowers[m_CurrentSequenceIndex])
		PointTower->SetPointTowerState(EPointTowerState::Active);

	/* 다음 거점 먹으라는 표기 Multicast로 쏴주기 */
	GAME_LV_GAME_MODE(this)->GetGameOverChecker()->Multicast_ShowMainInformConqueringPointTower();
	
	/* ZombieSpawn 관련 Initing 처리 */
	
	// ============ 현재 Sequence SpawnArea 가져오기 ===============

	TArray<AC_SpawnArea*> CurrentSpawnAreas = GetCurrentSequenceSpawnAreas();

	if (CurrentSpawnAreas.IsEmpty())
	{
		UC_Util::Print("[UC_PointTowerManager::StartActivateCurrentPointsSequence] : Current Sequence SpawnArea not found", FColor::Red, 10.f);
		return;
	}

	// 현재 Sequence SpawnArea 활성화
	for (AC_SpawnArea* SpawnArea : CurrentSpawnAreas)
	{
		if (!IsValid(SpawnArea))
			continue;

		SpawnArea->SetEnabled(true);
	}

	// ========== 현재 Sequence WaveSetting ============

	const FZombieWaveSetting* WaveSetting = GetCurrentWaveSetting();

	if (!WaveSetting)
	{
		UC_Util::Print("[UC_PointTowerManager::StartActivateCurrentPointsSequence] : Current Sequence WaveSetting not found", FColor::Red, 10.f);
		return;
	}

	//========= Zombie Spawn 시작 ==================

	if (!IsValid(m_ZombieManager))
	{
		UC_Util::Print("[UC_PointTowerManager::StartActivateCurrentPointsSequence] : ZombieManager is nullptr", FColor::Red, 10.f);
		return;
	}

	if (!m_ZombieManager->StartSpawnLoop(CurrentSpawnAreas, *WaveSetting))
	{
		UC_Util::Print("[UC_PointTowerManager::StartActivateCurrentPointsSequence] : StartSpawnLoop Failed!!", FColor::Red, 10.f);
	}
}

const FZombieWaveSetting* UC_PointTowerManager::GetCurrentWaveSetting() const
{
	if (!m_PointTowers.IsValidIndex(m_CurrentSequenceIndex))
		return nullptr;

	const TSet<AC_PointTower*>& CurrentTowers = m_PointTowers[m_CurrentSequenceIndex];

	if (CurrentTowers.IsEmpty())
		return nullptr;

	for (AC_PointTower* PointTower : CurrentTowers)
	{
		if (!IsValid(PointTower))
			continue;

		return &PointTower->GetZombieWaveSetting();
	}

	return nullptr;
}

bool UC_PointTowerManager::RegisterPointTower(AC_PointTower* _PointTower)
{
	// 새로운 Sequence 신규 PointTower, size를 늘림과 동시에 넣어줌
	if (!m_PointTowers.IsValidIndex(_PointTower->m_ActivateSequenceIdx))
	{
		m_PointTowers.SetNum(_PointTower->m_ActivateSequenceIdx + 1);
		m_PointTowers[_PointTower->m_ActivateSequenceIdx].Add(_PointTower);
		return true;
	}

	// 이미 해당 Sequence가 등록된 상황에서 중복으로 들어가는지 체크
	TSet<AC_PointTower*>& TargetSeqSet = m_PointTowers[_PointTower->m_ActivateSequenceIdx]; 
	if (TargetSeqSet.Contains(_PointTower)) return false; 

	/* 동일 sequence에 여러 거점이 동시에 활성화될 수 있는 상황임 */
	// 이러한 경우, m_bCanDamagedAfterConquer값을 true로 두어,
	// 점령을 이미 한 거점인 경우에도 공격을 받아 Conquer 게이지가 떨어질 수 있게끔 처리한다
	
	TargetSeqSet.Add(_PointTower);

	for (AC_PointTower* PointTower : TargetSeqSet)
		PointTower->m_bCanDamagedAfterConquer = true;
	
	return true;
}

const TSet<AC_PointTower*>& UC_PointTowerManager::GetCurPointTowers() const
{
	if (!m_PointTowers.IsValidIndex(m_CurrentSequenceIndex)) return m_Dummy;
	return m_PointTowers[m_CurrentSequenceIndex];
}

void UC_PointTowerManager::OnPointTowerConquered()
{
	// 이번 Sequence가 모두 끝났는지 체크
	for (AC_PointTower* PointTower : m_PointTowers[m_CurrentSequenceIndex])
		if (PointTower->GetPointTowerState() == EPointTowerState::Active) return; // 아직 해당 Sequence의 모든 PointTower가 점령되지는 않은 상황 -> Continue

	/* 이번라운드가 실제로 끝난 상황 */
	
	for (AC_PointTower* PointTower : m_PointTowers[m_CurrentSequenceIndex])
	{
		// 이번 라운드 끝남 알림 Delegate 호출 및 비우기 처리
		// 해당 Tower를 감지한 Enemy의 감지 Container에서 이 Tower를 빼는 처리를 하기 위함(Mainly) -> 다른 처리를 넣어도 무방
		PointTower->m_OnCurPointTowerSequenceOver.Broadcast();
		PointTower->m_OnCurPointTowerSequenceOver.Clear();
	}
	
	/* TODO : 라운드 끝나고 이전 라운드에 배치되어 있는 스폰 지점 비활성화 처리할 것 */
	if (IsValid(m_ZombieManager))
	{
		m_ZombieManager->StopSpawnLoop();
	}

	// 현재 Sequence SpawnArea 비활성화
	for (AC_SpawnArea* SpawnArea : GetCurrentSequenceSpawnAreas())
	{
		if (!IsValid(SpawnArea))
			continue;

		SpawnArea->SetEnabled(false);
	}

	// 다음 라운드로 넘기기
	++m_CurrentSequenceIndex;
	
	
	// 게임오버 체크
	if (!m_PointTowers.IsValidIndex(m_CurrentSequenceIndex))
	{
		// 게임 오버 처리할 것 -> PlayerWin
		
		// 피격을 당해도 쓰러지지 않게끔 처리 (이미 쓰러진 플레이어는 그냥 누워있으셈) (서버 쪽 환경만 Immoratl 설정을 해주면 알아서 데미지 환산 때 서버 쪽에서 무적 걸림)
		for (AC_BasicPlayer* Player : LEVEL_MANAGER->GetPlayers())
			Player->GetStatComponent()->SetImmortal(); 

		GAME_LV_GAME_MODE(this)->GetGameOverChecker()->Multicast_GameOver(true);
		return;
	}
	
	// 아직 GameOver되지 않은 상황 -> 다음 라운드 진행
	StartActivateCurrentPointsSequence();
}

bool UC_PointTowerManager::RegisterSpawnArea(AC_SpawnArea* _SpawnArea)
{
	if (!IsValid(_SpawnArea))
		return false;

	const uint8 SequenceIdx = _SpawnArea->GetActivateSequenceIdx();

	// 만약에 배열 크기가 부족하면 Sequence에 맞게 확장
	if (!m_SpawnArea.IsValidIndex(SequenceIdx))
	{
		m_SpawnArea.SetNum(SequenceIdx + 1);
	}

	TSet<AC_SpawnArea*>& TargetArea = m_SpawnArea[SequenceIdx];

	// 중복 등록 방지
	if (TargetArea.Contains(_SpawnArea))
		return false;

	TargetArea.Add(_SpawnArea);

	return true;
}

TArray<AC_SpawnArea*> UC_PointTowerManager::GetCurrentSequenceSpawnAreas() const
{
	TArray<AC_SpawnArea*> Result;

	if (!m_SpawnArea.IsValidIndex(m_CurrentSequenceIndex))
		return Result;

	for (AC_SpawnArea* SpawnArea : m_SpawnArea[m_CurrentSequenceIndex])
	{
		if (!IsValid(SpawnArea))
			continue;

		Result.Add(SpawnArea);
	}

	return Result;
}
