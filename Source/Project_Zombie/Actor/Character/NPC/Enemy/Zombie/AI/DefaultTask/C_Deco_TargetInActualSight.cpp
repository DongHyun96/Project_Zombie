// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Deco_TargetInActualSight.h"

#include "Actor/Character/NPC/Enemy/Zombie/Controller/C_ZombieController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Utility/C_Util.h"

bool UC_Deco_TargetInActualSight::CalculateRawConditionValue(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMem) const
{
	AC_ZombieController* ZombieController = Cast<AC_ZombieController>(_OwnCom.GetAIOwner());
	if (!ZombieController)
	{
		UC_Util::Print("From UC_Deco_TargetInActualSight::CalculateRawConditionValue : ZombieController casting failed!", FColor::Red, 10.f);
		return false;
	}

	AActor* CurrentTarget = Cast<AActor>(_OwnCom.GetBlackboardComponent()->GetValueAsObject(m_Target.SelectedKeyName));
	return ZombieController->IsCurrentlyOnSight(CurrentTarget);
}
