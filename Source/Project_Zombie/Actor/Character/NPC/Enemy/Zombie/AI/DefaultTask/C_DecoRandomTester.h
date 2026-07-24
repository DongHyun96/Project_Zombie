// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "C_DecoRandomTester.generated.h"

/**
 *  지정한 랜덤 확률 통과 시 이 Decorator 검사 통과
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_DecoRandomTester : public UBTDecorator
{
	GENERATED_BODY()

public:
	
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMem) const override;
	
protected:

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), DisplayName = "SuccessChance")
	float m_SuccessChance{};
	
};
