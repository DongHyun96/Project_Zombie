// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "C_Task_Attack.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API UC_Task_Attack : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector	m_Target;

public:
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory) override;
	
protected:
	
	virtual void TickTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds) override;

public:
	UC_Task_Attack();
};
