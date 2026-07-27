// Fill out your copyright notice in the Description page of Project Settings.


#include "C_InteractionStrategyBase.h"

#include "Actor/Character/Player/C_BasicPlayer.h"


bool UC_InteractionStrategyBase::CanStartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const
{
	if (!_Interactor || !_TargetActor)
	{
		return false;
	}

	if (_Interactor == _TargetActor)
	{
		return false;
	}

	return true;
}


bool UC_InteractionStrategyBase::StartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	return CanStartInteraction(_Interactor, _TargetActor);
}

void UC_InteractionStrategyBase::CancleInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
}

void UC_InteractionStrategyBase::CompleteInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
}
