// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PointTowerManager.h"

#include "Actor/PointTower/C_PointTower.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Utility/C_Util.h"

UC_PointTowerManager::UC_PointTowerManager()
{
	
}

void UC_PointTowerManager::OnWorldBeginPlay()
{
	m_CurrentSequenceIndex = 0;
	
	// For Testing
	GetWorld()->GetTimerManager().SetTimer
	(
		m_TestTimerHandle, this, &UC_PointTowerManager::StartActivateCurrentPointsSequence, 2.5f, false
	);
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
}

bool UC_PointTowerManager::RegisterPointTower(AC_PointTower* _PointTower)
{
	PRINT_LOCAL(GetWorld(), "[UC_PointTowerManager::RegisterPointTower]", FColor::Red, 10.f);
	
	// 새로운 Sequence 신규 PointTower, size를 늘림과 동시에 넣어줌
	if (!m_PointTowers.IsValidIndex(_PointTower->m_ActivateSequenceIdx) ||
		m_PointTowers[_PointTower->m_ActivateSequenceIdx].IsEmpty())
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
	
	// 다음 라운드로 넘기기
	++m_CurrentSequenceIndex;
	
	
	// 게임오버 체크
	if (!m_PointTowers.IsValidIndex(m_CurrentSequenceIndex))
	{
		// TODO : 게임 오버 처리할 것
		return;
	}
	
	// 아직 GameOver되지 않은 상황 -> 다음 라운드 진행
	StartActivateCurrentPointsSequence();
}
