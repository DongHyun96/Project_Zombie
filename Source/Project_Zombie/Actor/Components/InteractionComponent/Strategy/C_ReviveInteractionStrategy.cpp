// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ReviveInteractionStrategy.h"

#include "../../../Character/Player/C_BasicPlayer.h"
#include "../C_InteractionComponent.h"



UC_ReviveInteractionStrategy::UC_ReviveInteractionStrategy()
{
	m_InteractionDuration = 3.0f;
}

bool UC_ReviveInteractionStrategy::CanStartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const
{
	return false;
}

bool UC_ReviveInteractionStrategy::CanContinueInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const
{
	return false;
}

void UC_ReviveInteractionStrategy::StartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
}

void UC_ReviveInteractionStrategy::CancleInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
}

void UC_ReviveInteractionStrategy::CompleteInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
}
