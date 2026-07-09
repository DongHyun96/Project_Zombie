// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Task_Attack.h"
#include "../C_Zombie.h"
#include "../Controller/C_ZombieController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UC_Task_Attack::UC_Task_Attack()
{
	bCreateNodeInstance = false;

	// 매 프레임마다 TickTask 를 호출받을지 설정
	bNotifyTick = true;
}

EBTNodeResult::Type UC_Task_Attack::ExecuteTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory)
{
	Super::ExecuteTask(_OwnCom, _NodeMemory);

	return EBTNodeResult::Succeeded;
}

void UC_Task_Attack::TickTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds)
{
}
