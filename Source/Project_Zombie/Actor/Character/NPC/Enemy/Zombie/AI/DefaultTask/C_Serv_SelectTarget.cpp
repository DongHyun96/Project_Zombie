// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Serv_SelectTarget.h"

#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "Actor/Character/NPC/Enemy/Zombie/Controller/C_ZombieController.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/PointTower/C_PointTower.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameModeAndManager/C_GameMode_GameLv.h"
#include "GameModeAndManager/GameLevelmanager/C_GameLevelManager.h"
#include "GameModeAndManager/PointTowerManager/C_PointTowerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"

UC_Serv_SelectTarget::UC_Serv_SelectTarget()
{
	// 간격
	Interval = 0.3f;
	// 랜덤편차
	RandomDeviation = 0.1f;
}

void UC_Serv_SelectTarget::TickNode(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DT)
{
	Super::TickNode(_OwnCom, _NodeMemory, _DT);

	AC_ZombieController* pController = Cast< AC_ZombieController>(_OwnCom.GetAIOwner());
	if (!pController)
		return;

	AC_Zombie* pZombie = Cast<AC_Zombie>(pController->GetPawn());
	if (!pZombie)
		return;

	// 인지범위를 벗어난 대상이 일정시간이 지나면 인지목록에서 제거
	pController->ClearSensedTarget(3.f);

	float MaxAggro      = -1.f;
	AActor* pBestTarget = nullptr;
	FVector Pos{};

	// 컨트롤러가 인지한 대상중 가장 적절한 대상을 골라서 블랙보드 Target 에 업로드
	for (const FSensedTargetInfo& Info : pController->GetSensedTargets())
	{
		if (!Info.Target.IsValid()) continue;
		
		// 감지된 PointTower인 경우에, 현재 공격 가능한 상황인지 따짐 -> 공격 불가능한 타워의 경우 넘어감
		if (AC_PointTower* PointTower = Cast<AC_PointTower>(Info.Target))
			if (!PointTower->CanCurrentlyAttackedByZombie()) continue;

		// 어그로 수치 먼저 판단
		if (MaxAggro < Info.AggroValue)
		{
			MaxAggro    = Info.AggroValue;
			pBestTarget = Info.Target.Get();
			Pos         = Info.Target->GetActorLocation();
		}

		// 어그로가 동일한 경우
		else if (MaxAggro == Info.AggroValue)
		{
			const float DistSqrOrigin = FVector::DistSquared(pZombie->GetActorLocation(), Pos);
			const float DistSqrNew    = FVector::DistSquared(pZombie->GetActorLocation(), Info.Target->GetActorLocation());

			if (DistSqrNew < DistSqrOrigin)
			{
				pBestTarget = Info.Target.Get();
				Pos         = Info.Target->GetActorLocation();
			}
		}
	}
	
	if (pBestTarget) // BestTarget가 나온 상황
	{
		// BestTarget을 Target으로 지정
		UBlackboardComponent* pBBCom = _OwnCom.GetBlackboardComponent();
		if (!pBBCom) return;

		// 기존에는 타겟이 없었는데
		// 이번에 처음 추격할 타겟이 생긴 경우
		UObject* CurrentTarget =
			pBBCom->GetValueAsObject(m_Target.SelectedKeyName);

		if (!IsValid(CurrentTarget))
		{
			pZombie->StartChaseSoundLoop();
		}
	
		pBBCom->SetValueAsObject(m_Target.SelectedKeyName, pBestTarget);
		return;
	}

	// 여기까지 왔다는 것은 현재 추격할 Target을 찾지 못한 상황
	UBlackboardComponent* pBBCom = _OwnCom.GetBlackboardComponent();
	if (!pBBCom)
		return;

	// 추격음 반복 정지
	pZombie->StopChaseSoundLoop();

	// 기존 Blackboard Target도 제거
	pBBCom->ClearValue(m_Target.SelectedKeyName);

	// BestTarget이 나오지 않은 상황
	//  -> 가장 가까운 플레이어 또는 거점을 찾는다
	/*float MinDistSqr = FLT_MAX;

	if (!m_GameLevelManager)
	{
		m_GameLevelManager = GetWorld()->GetSubsystem<UC_GameLevelManager>();
		if (!m_GameLevelManager) return;
	}
	
	for (AC_BasicPlayer* pPlayer : m_GameLevelManager->GetPlayers())
	{
		if (!pPlayer) continue;

		const float DistSqr = FVector::DistSquared(pPlayer->GetActorLocation(), pZombie->GetActorLocation());

		if (DistSqr < MinDistSqr)
		{
			MinDistSqr  = DistSqr;
			pBestTarget = pPlayer;
		}
	}

	// 현재 sequence의 PointTower들 또한 확인
	for (AC_PointTower* PointTower : POINT_TOWER_MANAGER(pZombie)->GetCurPointTowers())
	{
		const float DistSqr = FVector::DistSquared(PointTower->GetActorLocation(), pZombie->GetActorLocation());

		if (DistSqr < MinDistSqr)
		{
			MinDistSqr  = DistSqr;
			pBestTarget = PointTower;
		}
	}

	// 가장 가까운 타겟을 블랙보드에 타겟으로 설정
	UBlackboardComponent* pBBCom = _OwnCom.GetBlackboardComponent();
	if (!pBBCom) return;

	// 실제 타겟을 찾았을 때만
	if (IsValid(pBestTarget))
	{
		UObject* CurrentTarget =
			pBBCom->GetValueAsObject(m_Target.SelectedKeyName);

		// 기존 타겟이 없었다면 추격 시작 사운드
		if (!IsValid(CurrentTarget))
		{
			pZombie->Multicast_PlayChaseSound();
		}
	}
	
	pBBCom->SetValueAsObject(m_Target.SelectedKeyName, pBestTarget);*/
}
