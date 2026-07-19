// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Deco_IsInRange.h"

#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "Actor/Character/NPC/Enemy/Zombie/Controller/C_ZombieController.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Utility/C_Util.h"

UC_Deco_IsInRange::UC_Deco_IsInRange()
{
	NodeName = TEXT("Is In Range");

	// 데코레이터의 TickNode가 호출되도록 설정
	bNotifyTick = true;
}

bool UC_Deco_IsInRange::CalculateRawConditionValue(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMem) const
{
	AC_ZombieController* pController = Cast<AC_ZombieController>(_OwnCom.GetAIOwner());
	if (!pController)
		return false;

	AC_Zombie* pZombie = Cast<AC_Zombie>(pController->GetPawn());
	if (!pZombie)
		return false;

	UBlackboardComponent* pBBCom = _OwnCom.GetBlackboardComponent();
	if (!pBBCom)
		return false;

	AActor* pTargetActor = Cast<AActor>(pBBCom->GetValueAsObject(m_Target.SelectedKeyName));

	float Dist = FVector::Dist(pZombie->GetActorLocation(), pTargetActor->GetActorLocation());

	UC_EnemySkillComponent* SkillCom = pZombie->GetSkillComponent();

	// TODO : 에디터 BehaviorTree 에서 해당 SkillSlot으로 초기화해줄 것 (기본은 1번 Slot으로 되어있음)
	float AttackRange = SkillCom->GetSkillRange(m_SkillSlot);

	return Dist <= AttackRange;
}

void UC_Deco_IsInRange::TickNode(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds)
{
	Super::TickNode(_OwnCom, _NodeMemory, _DeltaSeconds);

	UC_Util::Print("TickNode");

	ConditionalFlowAbort(_OwnCom, EBTDecoratorAbortRequest::ConditionResultChanged);
}

