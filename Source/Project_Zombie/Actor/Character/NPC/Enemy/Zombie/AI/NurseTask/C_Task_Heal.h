// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/NPC/Enemy/Zombie/AI/DefaultTask/C_Task_UseSkill.h"
#include "BehaviorTree/BTTaskNode.h"
#include "C_Task_Heal.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_Task_Heal : public UC_Task_UseSkill
{
	GENERATED_BODY()

public:
	
	UC_Task_Heal();

public:
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	
	virtual void TickTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds) override;
	
public:
	
	/// <summary>
	/// Projectile Heal 스폰 Interval 시간 재기 위한 float node memory
	/// </summary>
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(float); }
	
};
