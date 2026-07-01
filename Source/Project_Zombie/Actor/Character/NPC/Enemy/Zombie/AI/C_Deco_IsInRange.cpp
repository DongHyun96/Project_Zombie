// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Deco_IsInRange.h"
#include "../Controller/C_ZombieController.h"
#include "../C_Zombie.h"

#include "BehaviorTree/BlackboardComponent.h"

UC_Deco_IsInRange::UC_Deco_IsInRange()
{
}

bool UC_Deco_IsInRange::CalculateRawConditionValue(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMem) const
{
	AC_ZombieController* pController = Cast<AC_ZombieController>(_OwnCom.GetAIOwner());
	if (!pController)
		return false;

	AC_Zombie* pZombie = Cast<AC_Zombie>(pController->GetPawn());
	if (!pZombie)
		return false;

	UBlackboardComponent* pBBCom = _OwnCom.GetBlackboardComponent();
	if (!pBBCom)
		return false;

	AActor* pTargetActor = Cast<AActor>(pBBCom->GetValueAsObject(m_Target.SelectedKeyName));

	float Dist = FVector::Dist(pZombie->GetTargetLocation(), pTargetActor->GetActorLocation());

	return (Dist < 200.f);
}

