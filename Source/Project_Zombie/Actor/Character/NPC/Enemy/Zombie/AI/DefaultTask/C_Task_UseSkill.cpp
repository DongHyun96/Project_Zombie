// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Task_UseSkill.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

#include "AIController.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"
#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"


UC_Task_UseSkill::UC_Task_UseSkill()
{
	bCreateNodeInstance = false;

	// 매 프레임마다 TickTask 를 호출받을지 설정
	//bNotifyTick = true;
}

EBTNodeResult::Type UC_Task_UseSkill::ExecuteTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory)
{
	Super::ExecuteTask(_OwnCom, _NodeMemory);

	// Task 를 실행중인 Controller 를 가져옴
	AAIController* pController = _OwnCom.GetAIOwner();

	if (!pController)
		return EBTNodeResult::Failed;

	// Controller 가 빙의한 Pawn 을 가져옴
	AC_Zombie* pZombie = Cast<AC_Zombie>(pController->GetPawn());

	if (!pZombie)
		return EBTNodeResult::Failed;

	// 블랙보드 가져오기
	UBlackboardComponent* Blackboard = _OwnCom.GetBlackboardComponent();

	if (!Blackboard)
		return EBTNodeResult::Failed;

	// 블랙보드 타겟 가져오기
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(m_Target.SelectedKeyName));

	if(!Target)
		return EBTNodeResult::Failed;

	// 이동정지
	pController->StopMovement();

	// 타겟 방향 계산하기
	FVector TargetDirection = Target->GetActorLocation() - pZombie->GetActorLocation();

	TargetDirection.Z = 0.f;

	if (!TargetDirection.IsNearlyZero())
	{
		const FRotator TargetRotation = TargetDirection.Rotation();

		pZombie->SetActorRotation(FRotator(0.f, TargetRotation.Yaw, 0.f));
	}

	// 스킬 컴포넌트 가져오기
	UC_EnemySkillComponent* pSkillCom = pZombie->GetComponentByClass<UC_EnemySkillComponent>();

	if (!pSkillCom)
		return EBTNodeResult::Failed;

	// 스킬 사용 시도
	if (!pSkillCom->UseSkill(m_SkillSlot))
		return EBTNodeResult::Failed; // 실패 시, 이 Task Failed 처리
	
	// 스킬 사용 성공 (Activate 성공)
	pSkillCom->m_SkillEndDelegate.AddUObject(this, &UC_Task_UseSkill::OnSkillEnd, &_OwnCom);

	return EBTNodeResult::InProgress;
}

void UC_Task_UseSkill::TickTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds)
{
}

void UC_Task_UseSkill::OnSkillEnd(AC_BasicEnemy* _SkillUser, UBehaviorTreeComponent* _BTCom)
{
	if (!_SkillUser || !_BTCom)
		return;

	UC_EnemySkillComponent* pSkillCom = _SkillUser->GetComponentByClass<UC_EnemySkillComponent>();

	if (pSkillCom)
	{
		pSkillCom->m_SkillEndDelegate.RemoveAll(this);
	}

	FinishLatentTask(*_BTCom, EBTNodeResult::Succeeded);
}
