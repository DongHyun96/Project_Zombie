// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Serv_NurseSelectMainAction.h"

#include "AIController.h"
#include "Actor/Character/NPC/Enemy/Zombie/NurseZombie/C_NurseZombie.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Utility/C_Util.h"

UC_Serv_NurseSelectMainAction::UC_Serv_NurseSelectMainAction()
{
	Interval        = 3.f; // 간격
	RandomDeviation = 0.5f; // 랜덤편차
	
	bNotifyBecomeRelevant = true;
}

void UC_Serv_NurseSelectMainAction::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	 
	m_MustAttackDistLimitSqr = m_MustAttackDistanceLimit * m_MustAttackDistanceLimit;
}

void UC_Serv_NurseSelectMainAction::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AC_NurseZombie* Nurse = Cast<AC_NurseZombie>(OwnerComp.GetAIOwner()->GetPawn());
	
	if (!Nurse)
	{
		UC_Util::Print("From UC_Serv_NurseSelectMainAction::TickNode : This Service is for Nurse Behavior tree", FColor::Red, 10.f);
		return;
	}
	
	/* Nurse MainAction 갈래 정하기 */

	UObject* TargetObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(m_AttackTarget.SelectedKeyName);
	AActor* AttackTarget = Cast<AActor>(TargetObject); 
	const TArray<AC_BasicEnemy*>& HealTargets = Nurse->GetHealProjectileTargets();

	/* 공격대상도 없고, HealTarget도 모두 없는 상황 (인게임에서 이러한 상황은 나오지 않아야 맞긴 함) */
	if (HealTargets.IsEmpty() && !AttackTarget)
	{
		// Default idle (아무 행동도 취하지 않는 Wait 상태) 로 처리
		SetNurseActionState(Nurse, OwnerComp.GetBlackboardComponent(), ENurseZombieActionState::Idle);
		return;
	}

	/* AttackTarget만 존재, HealTarget이 없는 경우 */
	if (HealTargets.IsEmpty())
	{
		// AttackAction으로 세팅
		SetNurseActionState(Nurse, OwnerComp.GetBlackboardComponent(), ENurseZombieActionState::Attack);
		return;
	}
	
	/* HealTarget들만 존재, AttackTarget이 없는 경우 */
	if (!AttackTarget)
	{
		// HealAction으로 세팅
		SetNurseActionState(Nurse, OwnerComp.GetBlackboardComponent(), ENurseZombieActionState::Healing);
		return;
	}

	/* HealTarget AttackTarget 모두 존재하는 상황 */
	
	// Priority 0 -> 힐이고 나발이고 AttackTarget이 무조건 공격을 해야하는 범위안에 잡혔을 때 (7.5m 이내 거리에 AttackTarget이 있는 경우)
	const float AttackTargetDistSqr = FVector::DistSquared(AttackTarget->GetActorLocation(), Nurse->GetActorLocation());
	
	if (AttackTargetDistSqr < m_MustAttackDistLimitSqr)
	{
		// AttackAction으로 세팅
		SetNurseActionState(Nurse, OwnerComp.GetBlackboardComponent(), ENurseZombieActionState::Attack);
		return;
	}

	// Priority 1 -> 힐을 줘야하는 대상이 두 명 이상일 경우, 힐 처리
	if (HealTargets.Num() > 1)
	{
		SetNurseActionState(Nurse, OwnerComp.GetBlackboardComponent(), ENurseZombieActionState::Healing);
		return;
	}

	// Priority 2 -> 힐을 줘야하는 대상이 1명 vs 공격대상 1명 -> 거리를 비교하여, 거리가 더 가까운 대상으로 ActionState 지정
	// 힐 Target과 AttackTarget간의 거리를 비교 -> 이거 근데 잘못하면 ActionState 진동할 수 있음
	const float HealTargetDistSqr = FVector::DistSquared(HealTargets[0]->GetActorLocation(), Nurse->GetActorLocation());
	
	SetNurseActionState
	(
		Nurse,
		OwnerComp.GetBlackboardComponent(),
		(AttackTargetDistSqr > HealTargetDistSqr) ? ENurseZombieActionState::Attack : ENurseZombieActionState::Healing
	);
}

void UC_Serv_NurseSelectMainAction::SetNurseActionState(AC_NurseZombie* _NurseZombie, UBlackboardComponent* _BBCom, ENurseZombieActionState _ActionState)
{
	// 이미 기존 값과 동일한 경우
	if (_ActionState == _NurseZombie->GetActionState()) return;

	_NurseZombie->SetActionState(_ActionState);
	_BBCom->SetValueAsEnum(m_ActionState.SelectedKeyName, static_cast<uint8>(_ActionState));
}
