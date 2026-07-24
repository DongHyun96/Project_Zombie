// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"

#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"

#include "C_Deco_CanUseSkill.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API UC_Deco_CanUseSkill : public UBTDecorator
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Skill")
	ESkillSlot m_SkillSlot = ESkillSlot::Skill_1;

public:
	virtual void TickNode(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& _OwnerCom, uint8* _NodeMemory) const override;

public:
	UC_Deco_CanUseSkill();
	
};
