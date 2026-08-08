// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "C_Deco_IsPlayerTarget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_Deco_IsPlayerTarget : public UBTDecorator
{
	GENERATED_BODY()

public:
	
	UC_Deco_IsPlayerTarget();
	
public:
	
	virtual void TickNode(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& _OwnerCom, uint8* _NodeMemory) const override;
	
protected:
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector	m_Target{};

};
