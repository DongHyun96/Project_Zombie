// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Components/InteractionComponent/Strategy/C_PlayerStatUpWidgetStrategy.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Controller/C_BasicPlayerController.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/InvenUI/Upgrade/C_PlayerStatUpgradeWidget.h"


UC_PlayerStatUpWidgetStrategy::UC_PlayerStatUpWidgetStrategy()
{
	m_InteractionDuration = 0;
}

bool UC_PlayerStatUpWidgetStrategy::CanStartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const
{
	return Super::CanStartInteraction(_Interactor, _TargetActor);
}

bool UC_PlayerStatUpWidgetStrategy::StartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	if (!CanStartInteraction(_Interactor, _TargetActor))
		return false;
	
	// 여기서 Updrade WIdget 띄워주기.
	AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(_Interactor->GetController());
	
	if (!PC) return false;
	
	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	
	if (!UIManager) return false;
	
	UC_InventoryWidget* InvenWidget = UIManager->GetInventoryWidget();
	
	if (!InvenWidget) return false;
	
	UC_PlayerStatUpgradeWidget* PlayerStatUpgradeWidget = InvenWidget->GetPlayerStatUpgradeWidget();
	
	if (!PlayerStatUpgradeWidget) return false;
	
	PlayerStatUpgradeWidget->SetUsePlayer(_Interactor);
	
	_Interactor->ToggleInventoryWidget();
		
	// 장비위젯 | 업그레이드위젯 | Player Inven Widget 을 보여줌(Visible) 
	InvenWidget->ShowPlayerStatUpgradeWidget();
	
	return true;
}

void UC_PlayerStatUpWidgetStrategy::CancleInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	Super::CancleInteraction(_Interactor, _TargetActor);
	
	// 여기서 Updrade WIdget 띄워주기.
	AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(_Interactor->GetController());
	
	if (!PC) return;
	
	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	
	if (!UIManager) return;
	
	UC_InventoryWidget* InvenWidget = UIManager->GetInventoryWidget();
	
	if (!InvenWidget) return;
	
	UC_PlayerStatUpgradeWidget* PlayerStatUpgradeWidget = InvenWidget->GetPlayerStatUpgradeWidget();
	
	if (!PlayerStatUpgradeWidget) return;
	
	PlayerStatUpgradeWidget->InitWidget();
	
	InvenWidget->CloseItemUpgradeWidget();
}

void UC_PlayerStatUpWidgetStrategy::CompleteInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	Super::CompleteInteraction(_Interactor, _TargetActor);
}