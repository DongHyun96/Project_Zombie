// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/NPC/Enemy/Zombie/AI/DefaultTask/C_Task_UseSkill.h"
#include "C_Task_GrabWeapon.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_Task_GrabWeapon : public UC_Task_UseSkill
{
	GENERATED_BODY()

public:
	
	UC_Task_GrabWeapon();
	
public:
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	virtual void TickTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds) override;
	
	virtual void OnTaskFinished
	(
		UBehaviorTreeComponent& OwnerComp,
		uint8*					NodeMemory,
		EBTNodeResult::Type 	TaskResult
	) override;

protected:
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector m_MainState{};
	
};
