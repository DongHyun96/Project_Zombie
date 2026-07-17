// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Task_UseSkill.h"

#include "AIController.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"
#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "Utility/C_Util.h"


UC_Task_UseSkill::UC_Task_UseSkill()
{
	bCreateNodeInstance = false;

	// 매 프레임마다 TickTask 를 호출받을지 설정
	//bNotifyTick = true;
	
	// TaskFinished 호출을 받을지 결정
	bNotifyTaskFinished = true;
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

	// 스킬 사용 시도
	if (!pSkillCom->UseSkill(m_SkillSlot))
		return EBTNodeResult::Failed; // 실패 시, 이 Task Failed 처리
	
	// 스킬 사용 성공 (Activate 성공)
	pSkillCom->m_SkillEndDelegate.AddUObject(this, &UC_Task_UseSkill::OnSkillEnd, &_OwnCom);
	UE_LOG(LogTemp, Warning, TEXT("Delegate Add"));


	return EBTNodeResult::InProgress;
}

void UC_Task_UseSkill::TickTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds)
{
}

void UC_Task_UseSkill::OnTaskFinished
(
	UBehaviorTreeComponent& OwnerComp,
	uint8*					NodeMemory,
	EBTNodeResult::Type		TaskResult
)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
	
	const FColor DebugColor = FColor::MakeRandomColor();
	UC_Util::Print("OnTaskFinished", DebugColor, 30.f);

	// Abort 처리로 Task가 끝나지 않은 상황
	if (TaskResult != EBTNodeResult::Aborted) return;
	
	// Abort 처리로 Task가 끝난 상황
	UC_Util::Print("OnTaskFinished By Aborted", DebugColor, 30.f);
	
	AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Enemy)
	{
		UC_Util::Print("From UC_Task_UseSkill::OnTaskFinished : Enemy castig failed!", FColor::Red, 10.f);
		return;
	}
	
	// 해당 Skill이 아직 발동이 되는 중임 -> 이걸 직접 끊어주어야 한다
	Enemy->GetSkillComponent()->EndSkillManually();
}

void UC_Task_UseSkill::OnSkillEnd(AC_BasicEnemy* _SkillUser, UBehaviorTreeComponent* _BTCom)
{
	if (_SkillUser && _BTCom)
	{
		UC_EnemySkillComponent* pSkillCom = _SkillUser->GetComponentByClass<UC_EnemySkillComponent>();
		pSkillCom->m_SkillEndDelegate.RemoveAll(this);
	}

	FinishLatentTask(*_BTCom, EBTNodeResult::Succeeded);
}
