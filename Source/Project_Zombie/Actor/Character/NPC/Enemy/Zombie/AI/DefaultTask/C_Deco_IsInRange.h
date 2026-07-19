// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "C_Deco_IsInRange.generated.h"

enum class ESkillSlot : uint8;

// 공격 사거리에 타겟이 있는지 확인하는 데코레이션
UCLASS()
class PROJECT_ZOMBIE_API UC_Deco_IsInRange : public UBTDecorator
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector	m_Target;

	// IsInRange 검사 대상 스킬슬롯 (해당 Slot에 장착된 SkillRange로 Range검사)
	UPROPERTY(EditAnywhere, Category = "Skill")
	ESkillSlot	m_SkillSlot{};
	
public:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMem) const override;
	virtual void TickNode(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds) override;

public:
	UC_Deco_IsInRange();
};
