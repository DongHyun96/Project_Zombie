// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "C_Task_UseSkill.generated.h"

enum class ESkillSlot : uint8;

UCLASS()
class PROJECT_ZOMBIE_API UC_Task_UseSkill : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector m_Target;

	UPROPERTY(EditAnywhere, Category = "Skill")
	ESkillSlot	m_SkillSlot;

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds);

public:
	void OnSkillEnd(class AC_BasicEnemy* _SkillUser, UBehaviorTreeComponent* _BTCom);
	
public:
	UC_Task_UseSkill();
	
};
