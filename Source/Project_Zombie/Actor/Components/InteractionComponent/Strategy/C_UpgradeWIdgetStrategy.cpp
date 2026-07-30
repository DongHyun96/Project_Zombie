// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Components/InteractionComponent/Strategy/C_UpgradeWIdgetStrategy.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Controller/C_BasicPlayerController.h"
#include "GameModeAndManager/C_UIManager.h"

UC_UpgradeWIdgetStrategy::UC_UpgradeWIdgetStrategy()
{
	m_InteractionDuration = 0;
}

bool UC_UpgradeWIdgetStrategy::CanStartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const
{
	return Super::CanStartInteraction(_Interactor, _TargetActor);
}

bool UC_UpgradeWIdgetStrategy::StartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	// 여기서 Updrade WIdget 띄워주기.
	AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(_Interactor->GetController());
	
	//APlayerController* PC = Cast<APlayerController>(GetController());
	
	if (!PC) return false;
	
	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	
	if (!UIManager) return false;
	
	
	
	
	return Super::StartInteraction(_Interactor, _TargetActor);
}

void UC_UpgradeWIdgetStrategy::CancleInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	Super::CancleInteraction(_Interactor, _TargetActor);
}

void UC_UpgradeWIdgetStrategy::CompleteInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	Super::CompleteInteraction(_Interactor, _TargetActor);
}
