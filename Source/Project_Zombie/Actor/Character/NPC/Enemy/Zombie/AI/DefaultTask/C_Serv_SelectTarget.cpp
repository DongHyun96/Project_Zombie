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

	UBlackboardComponent* pBBCom = _OwnCom.GetBlackboardComponent();
	if (!pBBCom) return;

	// 인지범위를 벗어난 대상이 일정시간이 지나면 인지목록에서 제거
	pController->ClearSensedTarget(3.f);

	float MaxAggro = -1.f;
	AActor* pBestTarget = nullptr;
	FVector Pos{};

	// 컨트롤러가 인지한 대상중 가장 적절한 대상을 골라서 블랙보드 Target 에 업로드
	for (const FSensedTargetInfo& Info : pController->GetSensedTargets())
	{
		if (!Info.Target.IsValid()) continue;

		// 감지된 PointTower인 경우에, 현재 공격 가능한 상황인지 따짐 -> 공격 불가능한 타워의 경우 넘어감
		if (AC_PointTower* PointTower = Cast<AC_PointTower>(Info.Target))
			if (!PointTower->CanCurrentlyAttackedByZombie()) continue;

		// 감지된 Player 중 그로기 상태의 Player인 경우에도 넘어감
		if (AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(Info.Target))
			if (Player->IsDead()) continue;

		// 어그로 수치 먼저 판단
		if (MaxAggro < Info.AggroValue)
		{
			MaxAggro = Info.AggroValue;
			pBestTarget = Info.Target.Get();
			Pos = Info.Target->GetActorLocation();
		}

		// 어그로가 동일한 경우
		else if (MaxAggro == Info.AggroValue)
		{
			const float DistSqrOrigin = FVector::DistSquared(pZombie->GetActorLocation(), Pos);
			const float DistSqrNew = FVector::DistSquared(pZombie->GetActorLocation(), Info.Target->GetActorLocation());

			if (DistSqrNew < DistSqrOrigin)
			{
				pBestTarget = Info.Target.Get();
				Pos = Info.Target->GetActorLocation();
			}
		}
	}

	if (pBestTarget) // BestTarget가 나온 상황
	{
		// BestTarget을 Target으로 지정

		// 기존에는 타겟이 없었는데
		// 이번에 처음 추격할 타겟이 생긴 경우
		UObject* CurrentTarget = pBBCom->GetValueAsObject(m_Target.SelectedKeyName);

		if (!IsValid(CurrentTarget))
		{
			pZombie->StartChaseSoundLoop();
		}

		pBBCom->SetValueAsObject(m_Target.SelectedKeyName, pBestTarget);
		return;
	}

	// BestTarget이 나오지 않은 상황
	//  -> 가장 가까운 플레이어 또는 거점을 찾는다
	float MinDistSqr = FLT_MAX;

	if (!m_GameLevelManager)
	{
		m_GameLevelManager = GetWorld()->GetSubsystem<UC_GameLevelManager>();
		if (!m_GameLevelManager) return;
	}

	for (AC_BasicPlayer* pPlayer : m_GameLevelManager->GetPlayers())
	{
		if (!pPlayer) continue;
		if (pPlayer->IsDead()) continue; // 그로기 상태의 Player의 경우 넘어감

		const float DistSqr = FVector::DistSquared(pPlayer->GetActorLocation(), pZombie->GetActorLocation());

		if (DistSqr < MinDistSqr)
		{
			MinDistSqr = DistSqr;
			pBestTarget = pPlayer;
		}
	}

	// 현재 sequence의 PointTower들 또한 확인
	for (AC_PointTower* PointTower : POINT_TOWER_MANAGER(pZombie)->GetCurPointTowers())
	{
		// 현재 공격 불가능한 PointTower인 경우
		if (!PointTower->CanCurrentlyAttackedByZombie()) continue;
		
		const float DistSqr = FVector::DistSquared(PointTower->GetActorLocation(), pZombie->GetActorLocation());

		if (DistSqr < MinDistSqr)
		{
			MinDistSqr = DistSqr;
			pBestTarget = PointTower;
		}
	}

	// 가장 가까운 타겟을 블랙보드에 타겟으로 설정
	UObject* CurTarget = pBBCom->GetValueAsObject(m_Target.SelectedKeyName);

	if (IsValid(pBestTarget))
	{
		if (!IsValid(CurTarget))
		{
			pZombie->StartChaseSoundLoop();
		}

		pBBCom->SetValueAsObject(m_Target.SelectedKeyName, pBestTarget);

		return;
	}

	pZombie->StopChaseSoundLoop();

	pBBCom->ClearValue(m_Target.SelectedKeyName);
}