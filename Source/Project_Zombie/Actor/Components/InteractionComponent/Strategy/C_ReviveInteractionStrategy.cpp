// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ReviveInteractionStrategy.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "../C_InteractionComponent.h"

#include "Utility/C_Util.h"

UC_ReviveInteractionStrategy::UC_ReviveInteractionStrategy()
{
	m_InteractionDuration = 3.0f;
}

bool UC_ReviveInteractionStrategy::CanStartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const
{
	AC_BasicPlayer* TargetPlayer = Cast<AC_BasicPlayer>(_TargetActor);

	if (!_Interactor || !TargetPlayer)
		return false;

	// 다운되어 있어야지 상호작용 가능
	//return TargetPlayer->IsDead();

	return true;
}

bool UC_ReviveInteractionStrategy::StartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	UC_Util::Print("----------->StartInteraction", FColor::Red, 10.f);

	return false;
}

void UC_ReviveInteractionStrategy::CancleInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
}

void UC_ReviveInteractionStrategy::CompleteInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
}
