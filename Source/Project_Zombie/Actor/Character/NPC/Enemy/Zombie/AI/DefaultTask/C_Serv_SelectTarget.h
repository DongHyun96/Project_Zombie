// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "C_Serv_SelectTarget.generated.h"

/// <summary>
/// 인지 대상중 어그로 수치에 따라 추적할 타겟 고르는 서비스 
/// </summary>

UCLASS()
class PROJECT_ZOMBIE_API UC_Serv_SelectTarget : public UBTService
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector m_Target;

public:
	virtual void TickNode(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DT) override;

public:
	UC_Serv_SelectTarget();
	
};
