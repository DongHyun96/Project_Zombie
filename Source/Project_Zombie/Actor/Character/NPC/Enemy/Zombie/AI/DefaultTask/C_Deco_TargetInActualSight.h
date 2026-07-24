// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "C_Deco_TargetInActualSight.generated.h"

/**
 * 지정된 Target이 실질적인 시야에 들어와 있는 상태인지 체크
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_Deco_TargetInActualSight : public UBTDecorator
{
	GENERATED_BODY()

public:
	
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMem) const override;

protected:
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector m_Target{};
	
};
