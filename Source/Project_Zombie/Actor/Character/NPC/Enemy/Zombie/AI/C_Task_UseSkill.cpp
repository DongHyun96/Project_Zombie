// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Task_UseSkill.h"

#include "../C_Zombie.h"
#include "../Controller/C_ZombieController.h"

UC_Task_UseSkill::UC_Task_UseSkill()
{
	bCreateNodeInstance = false;

	// 매 프레임마다 TickTask 를 호출받을지 설정
	bNotifyTick = true;
}

EBTNodeResult::Type UC_Task_UseSkill::ExecuteTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory)
{
	Super::ExecuteTask(_OwnCom, _NodeMemory);

	// 초기화
	*reinterpret_cast<float*>(_NodeMemory) = 0.f;

	// Task 를 실행중인 Controller 를 가져옴
	AAIController* pController = _OwnCom.GetAIOwner();

	// Controller 가 빙의한 Pawn 을 가져옴
	AC_Zombie* pZombie = Cast<AC_Zombie>(pController->GetPawn());

	if (nullptr == pZombie)
		return EBTNodeResult::Failed;

	UC_EnemySkillComponent* pSkillCom = pZombie->GetComponentByClass<UC_EnemySkillComponent>();
	if (nullptr == pSkillCom)
		return EBTNodeResult::Failed;

	pSkillCom->UseSkill(m_SkillSlot);

	return EBTNodeResult::Succeeded;
}

void UC_Task_UseSkill::TickTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds)
{
}
