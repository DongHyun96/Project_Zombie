// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Task_FireWeapon.h"

#include "AIController.h"
#include "Actor/Character/NPC/Enemy/Zombie/CopZombie/C_CopZombie.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "BehaviorTree/BlackboardComponent.h"

/*EBTNodeResult::Type UC_Task_FireWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}

void UC_Task_FireWeapon::TickTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds)
{
	Super::TickTask(_OwnCom, _NodeMemory, _DeltaSeconds);
}*/

void UC_Task_FireWeapon::OnTaskFinished
(
	UBehaviorTreeComponent& OwnerComp,
	uint8*					NodeMemory,
	EBTNodeResult::Type		TaskResult
)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
	
	// 총알을 모두 소진한 경우, 무기를 다시 뱉어낸다

	AC_CopZombie* Cop = Cast<AC_CopZombie>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Cop) return;

	if (!Cop->GetEquippedGun()) return; // 아마 고장난 상황
	
	if (Cop->GetEquippedGun()->GetCurrentAmmo() > 0) return; // 아직 무기를 사용할 수 있는 상황(장탄수가 남은 상황)
	
	// 장탄수가 남지 않은 경우, 무기를 뱉어버린다
	Cop->DropWeapon();
	
	// MainState Idle로 회귀
	Cop->SetCopZombieState(ECopZombieState::Idle);
	
	// 제대로 장착 처리되었다면 MainState 키값 수정 (다른 Zombie는 Service에서 바꾸지만, 이 해당 키는 바로 바꿔주어야 해당 Task를 바로 실행)
	OwnerComp.GetBlackboardComponent()->SetValueAsEnum(m_CopZombieState.SelectedKeyName, static_cast<uint8>(ECopZombieState::Idle));
}
