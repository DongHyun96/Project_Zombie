// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "C_Serv_NurseSelectMainAction.generated.h"

enum class ENurseZombieActionState : uint8;
/**
 * Nurse가 어떤 MainAction을 취해야하는지 결정하는 Service (Interval 보통보다 길게 처리될 예정)
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_Serv_NurseSelectMainAction : public UBTService
{
	GENERATED_BODY()

public:
	
	UC_Serv_NurseSelectMainAction();
	
public:

	// 서비스가 활성화될 때 한번 (BeginPlay같은 느낌)
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	
	/// <summary>
	/// <para> Nurse 쪽 ActionState 값 및 BT ActionState BBKey값 일괄 setting </para>
	/// <para> 이전 상태값과 같다면, BB 키값 setting x </para>
	/// </summary>
	void SetNurseActionState(class AC_NurseZombie* _NurseZombie, UBlackboardComponent* _BBCom, ENurseZombieActionState _ActionState);
	
protected:

	UPROPERTY(EditAnywhere, Category = "MustAttackDistance")
	float m_MustAttackDistanceLimit = 750.f; // 7.5m 제한 (BT에서 값 수정 가능)
	
	UPROPERTY(EditAnywhere, Category = "Blaackboard")
	FBlackboardKeySelector m_ActionState{};
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector m_AttackTarget{};

private:
	
	float m_MustAttackDistLimitSqr{};
	
};
