// Fill out your copyright notice in the Description page of Project Settings.


#include "C_SkinWidgetStrategy.h"

bool UC_SkinWidgetStrategy::CanStartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const
{
	return false;
}

bool UC_SkinWidgetStrategy::StartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	return false;
}

void UC_SkinWidgetStrategy::CancleInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
}

void UC_SkinWidgetStrategy::CompleteInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{

}
