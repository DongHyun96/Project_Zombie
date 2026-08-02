// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Task_UseSkill.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

#include "AIController.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"
#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"
#include "Utility/C_Util.h"


UC_Task_UseSkill::UC_Task_UseSkill()
{
	bCreateNodeInstance = false;

	// 매 프레임마다 TickTask 를 호출받을지 설정
	bNotifyTick = true;
	
	// TaskFinished 호출을 받을지 결정
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UC_Task_UseSkill::ExecuteTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory)
{
	UE_LOG(LogTemp, Warning, TEXT("ExecuteTask Slot = %d"), (int32)m_SkillSlot);

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

	// 스킬 컴포넌트 가져오기
	UC_EnemySkillComponent* pSkillCom = pZombie->GetComponentByClass<UC_EnemySkillComponent>();

	if (!pSkillCom)
		return EBTNodeResult::Failed;

	// 스킬 사용 시도
	if (!pSkillCom->UseSkill(m_SkillSlot))
		return EBTNodeResult::Failed; // 실패 시, 이 Task Failed 처리
	
	/* 스킬 사용 성공 (Activate 성공) */
	
	// 타겟 방향으로 바로 회전하는 Skill이면 바로 회전 처리
	if (pSkillCom->GetCurSkillData().Get()->bRotateToTargetOnActivation)
	{
		FVector TargetDirection = Target->GetActorLocation() - pZombie->GetActorLocation(); // 타겟 방향 계산하기
		TargetDirection.Z = 0.f;

		if (!TargetDirection.IsNearlyZero())
		{
			const FRotator TargetRotation = TargetDirection.Rotation();
			pZombie->SetActorRotation(FRotator(0.f, TargetRotation.Yaw, 0.f));
		}
	}
	
	pSkillCom->m_SkillEndDelegate.AddUObject(this, &UC_Task_UseSkill::OnSkillEnd, &_OwnCom);

	return EBTNodeResult::InProgress;
}

void UC_Task_UseSkill::TickTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds)
{
	Super::TickTask(_OwnCom, _NodeMemory, _DeltaSeconds);

	/* Target을 향해 Skill에 부여된 RotateSpeed를 이용하여 Target을 향한 Rotate 처리 */
	
	AC_Zombie* Zombie = Cast<AC_Zombie>(_OwnCom.GetAIOwner()->GetPawn());

	// 현재 사용중인 Skill의 Data
	const UC_EnemySkillData* CurSkillData = Zombie->GetSkillComponent()->GetCurSkillData().Get();
	if (!CurSkillData) return;
	
	// 만약 회전 속도가 0이면 처리 x
	if (CurSkillData->RotateSpeed <= 0.f) return;
	
	// RotateSpeed에 맞추어 Target을 향해 회전한다
	AActor* Target = Cast<AActor>(_OwnCom.GetBlackboardComponent()->GetValueAsObject(m_Target.SelectedKeyName));
	if (!Target) return;
	
	FVector Direction = Target->GetActorLocation() - Zombie->GetActorLocation();
	Direction.Z = 0.f;
	Direction.Normalize();
	
	const FRotator TargetRot = FRotationMatrix::MakeFromX(Direction).Rotator();
	const FRotator ResultRot = FMath::RInterpConstantTo(Zombie->GetActorRotation(), TargetRot, _DeltaSeconds, CurSkillData->RotateSpeed);
	Zombie->SetActorRotation(ResultRot);
}

void UC_Task_UseSkill::OnTaskFinished
(
	UBehaviorTreeComponent& OwnerComp,
	uint8*					NodeMemory,
	EBTNodeResult::Type		TaskResult
)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	// Abort 처리로 Task가 끝나지 않은 상황 (Montage의 EndSkill Notify에 의한 스킬Task 종료처리를 하는 가장 Trivial한 case)
	if (TaskResult != EBTNodeResult::Aborted) return;
	
	// Abort 처리로 Task가 끝난 상황, 직접적으로 Skill을 끊어야 한다
	
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
	if (!_SkillUser || !_BTCom)
		return;

	UC_EnemySkillComponent* pSkillCom = _SkillUser->GetComponentByClass<UC_EnemySkillComponent>();

	if (pSkillCom)
	{
		pSkillCom->m_SkillEndDelegate.RemoveAll(this);
	}
	
	FinishLatentTask(*_BTCom, EBTNodeResult::Succeeded);
}
