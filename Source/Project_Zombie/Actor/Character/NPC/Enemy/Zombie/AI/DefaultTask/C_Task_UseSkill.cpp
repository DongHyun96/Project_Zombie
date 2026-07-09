// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Task_UseSkill.h"

#include "AIController.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"
#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"


UC_Task_UseSkill::UC_Task_UseSkill()
{
	bCreateNodeInstance = true;

	// 매 프레임마다 TickTask 를 호출받을지 설정
	//bNotifyTick = true;
}

EBTNodeResult::Type UC_Task_UseSkill::ExecuteTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory)
{
	Super::ExecuteTask(_OwnCom, _NodeMemory);

	AAIController* AI = _OwnCom.GetAIOwner();

	if (AI)
	{
		AI->StopMovement();
	}


	// Task 를 실행중인 Controller 를 가져옴
	AAIController* pController = _OwnCom.GetAIOwner();

	// Controller 가 빙의한 Pawn 을 가져옴
	AC_Zombie* pZombie = Cast<AC_Zombie>(pController->GetPawn());

	if (nullptr == pZombie)
		return EBTNodeResult::Failed;

	UC_EnemySkillComponent* pSkillCom = pZombie->GetComponentByClass<UC_EnemySkillComponent>();

	if (nullptr == pSkillCom)
		return EBTNodeResult::Failed;

	pSkillCom->m_SkillEndDelegate.AddUObject(this, &UC_Task_UseSkill::OnSkillEnd, &_OwnCom);

	UE_LOG(LogTemp, Warning, TEXT("Delegate Add"));


	pSkillCom->UseSkill(m_SkillSlot);


	return EBTNodeResult::InProgress;
}

void UC_Task_UseSkill::TickTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds)
{
}

void UC_Task_UseSkill::OnSkillEnd(AC_BasicEnemy* _SkillUser, UBehaviorTreeComponent* _BTCom)
{
	UE_LOG(LogTemp, Warning, TEXT("OnSkillEnd"));

	if (_SkillUser && _BTCom)
	{
		UC_EnemySkillComponent* pSkillCom = _SkillUser->GetComponentByClass<UC_EnemySkillComponent>();
		pSkillCom->m_SkillEndDelegate.RemoveAll(this);
	}

	FinishLatentTask(*_BTCom, EBTNodeResult::Succeeded);
}
