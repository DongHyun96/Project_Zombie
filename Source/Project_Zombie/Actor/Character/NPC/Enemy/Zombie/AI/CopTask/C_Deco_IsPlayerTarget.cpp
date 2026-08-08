// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Deco_IsPlayerTarget.h"

#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "Actor/Character/NPC/Enemy/Zombie/Controller/C_ZombieController.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"

UC_Deco_IsPlayerTarget::UC_Deco_IsPlayerTarget()
{
}

void UC_Deco_IsPlayerTarget::TickNode(UBehaviorTreeComponent& _OwnCom, uint8* _NodeMemory, float _DeltaSeconds)
{
	Super::TickNode(_OwnCom, _NodeMemory, _DeltaSeconds);
}

bool UC_Deco_IsPlayerTarget::CalculateRawConditionValue(UBehaviorTreeComponent& _OwnerCom, uint8* _NodeMemory) const
{
	/* 현재 Target이 Player인지 조사 */
	
	AC_ZombieController* pController = Cast<AC_ZombieController>(_OwnerCom.GetAIOwner());
	if (!pController)
		return false;

	AC_Zombie* pZombie = Cast<AC_Zombie>(pController->GetPawn());
	if (!pZombie)
		return false;

	UBlackboardComponent* pBBCom = _OwnerCom.GetBlackboardComponent();
	if (!pBBCom) return false;
		
	return Cast<AC_BasicPlayer>(pBBCom->GetValueAsObject(m_Target.SelectedKeyName)) != nullptr;
}
