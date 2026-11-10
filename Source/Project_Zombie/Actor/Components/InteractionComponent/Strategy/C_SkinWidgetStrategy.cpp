// Fill out your copyright notice in the Description page of Project Settings.


#include "C_SkinWidgetStrategy.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Controller/C_BasicPlayerController.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/SkinUI/C_SkinWidget.h"

UC_SkinWidgetStrategy::UC_SkinWidgetStrategy()
{
	m_InteractionDuration = 0.f;
}

bool UC_SkinWidgetStrategy::CanStartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const
{
	return Super::CanStartInteraction(_Interactor, _TargetActor);
}

bool UC_SkinWidgetStrategy::StartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	if (!CanStartInteraction(_Interactor, _TargetActor)) return false;

	AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(_Interactor->GetController());
	if (!PC) return false;

	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	if (!UIManager) return false;

	UC_SkinWidget* SkinWidget = UIManager->GetSkinWidget();
	if (!SkinWidget) return false;

	// SkinWidget 열기
	SkinWidget->SetUsePlayer(_Interactor);
	UIManager->ToggleMouseInputNeededWidgetVisibility(EMouseCursorNeededWidgetType::SkinWidget, true);

	return true;
}

void UC_SkinWidgetStrategy::CancleInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	Super::CancleInteraction(_Interactor, _TargetActor);

	if (!_Interactor || !_TargetActor) return;

	AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(_Interactor->GetController());
	if (!PC) return;

	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	if (!UIManager) return;

	UC_SkinWidget* SkinWidget = UIManager->GetSkinWidget();
	if (!SkinWidget) return;

	// SKinWidget 닫기
	UIManager->ToggleMouseInputNeededWidgetVisibility(EMouseCursorNeededWidgetType::SkinWidget, false);
}

void UC_SkinWidgetStrategy::CompleteInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{

}
