// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Serv_SelectTarget.h"

#include "../Controller/C_ZombieController.h"
#include "../C_Zombie.h"

#include "BehaviorTree/BlackboardComponent.h"

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

	float MaxAggro = -1.f;
	AActor* pBestTarget = nullptr;
	FVector Pos;

	// 컨트롤러가 인지한 대상중 가장 적절한 대상을 골라서 블랙보드 Target 에 업로드
	for (const FSensedTargetInfo& Info : pController->GetSensedTargets())
	{
		if (!Info.Target.IsValid())
			continue;

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
			float DistOrigin = FVector::Dist(pZombie->GetActorLocation(), Pos);
			float DistNew = FVector::Dist(pZombie->GetActorLocation(), Info.Target->GetActorLocation());

			if (DistNew < DistOrigin)
			{
				pBestTarget = Info.Target.Get();
				Pos = Info.Target->GetActorLocation();
			}
		}
	}

	UBlackboardComponent* pBBCom = _OwnCom.GetBlackboardComponent();
	if (!pBBCom)
		return;

	pBBCom->SetValueAsObject(m_Target.SelectedKeyName, pBestTarget);

}
