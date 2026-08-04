// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PointTowerManager.h"

#include "Actor/PointTower/C_PointTower.h"
#include "Utility/C_Util.h"

UC_PointTowerManager::UC_PointTowerManager()
{
	
}

void UC_PointTowerManager::OnWorldBeginPlay()
{	
	UC_Util::Print("PointTowerManager::OnWorldBeginPlay()", FColor::Red, 10.f);
}

void UC_PointTowerManager::StartFirstActivatePointsSequence()
{
	m_CurrentSequenceIndex = 0;

	// 첫 거점들 활성화
	for (AC_PointTower* PointTower : m_PointTowers[m_CurrentSequenceIndex])
		PointTower->Activate();
}

bool UC_PointTowerManager::RegisterPointTower(AC_PointTower* _PointTower)
{
	// 새로운 Sequence 신규 PointTower, size를 늘림과 동시에 넣어줌
	if (!m_PointTowers.IsValidIndex(_PointTower->m_ActivateSequenceIdx))
	{
		m_PointTowers.SetNum(_PointTower->m_ActivateSequenceIdx);
		m_PointTowers[_PointTower->m_ActivateSequenceIdx].Add(_PointTower);
		return true;
	}

	// 이미 해당 Sequence가 등록된 상황에서 중복으로 들어가는지 체크
	TSet<AC_PointTower*>& TargetSeqSet = m_PointTowers[_PointTower->m_ActivateSequenceIdx]; 
	if (TargetSeqSet.Contains(_PointTower)) return false; 

	// 동일 sequence에 여러 거점이 동시에 활성화될 수 있는 상황
	// 이러한 경우, m_bCanDamagedAfterConquer값을 true로 두어,
	// 점령을 이미 한 거점인 경우에도 공격을 받아 Conquer 게이지가 떨어질 수 있게끔 처리한다
	TargetSeqSet.Add(_PointTower);

	for (AC_PointTower* PointTower : TargetSeqSet)
		PointTower->m_bCanDamagedAfterConquer = true;
	
	return true;
}
