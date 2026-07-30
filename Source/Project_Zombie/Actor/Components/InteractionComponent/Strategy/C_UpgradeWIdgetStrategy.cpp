// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Components/InteractionComponent/Strategy/C_UpgradeWIdgetStrategy.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/InteractionComponent/C_InteractionComponent.h"
#include "Controller/C_BasicPlayerController.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/InvenUI/Upgrade/C_ItemUpgradeWidget.h"

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
	if (!CanStartInteraction(_Interactor, _TargetActor))
		return false;
	
	// 여기서 Updrade WIdget 띄워주기.
	AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(_Interactor->GetController());
	
	if (!PC) return false;
	
	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	
	if (!UIManager) return false;
	
	UC_InventoryWidget* InvenWidget = UIManager->GetInventoryWidget();
	
	if (!InvenWidget) return false;
	
	UC_ItemUpgradeWidget* ItemUpgradeWidget = InvenWidget->GetItemUpgradeWidget();
	
	if (!ItemUpgradeWidget) return false;
	
	ItemUpgradeWidget->SetUsePlayer(_Interactor);
	
	_Interactor->ToggleInventoryWidget();
		
	// 장비위젯 | 업그레이드위젯 | Player Inven Widget 을 보여줌(Visible) 
	InvenWidget->ShowUpgradeWidget();
	
	// TODO : CancelInteract 구현 보고 없애야 할 수 있음.
	_Interactor->GetInteractionComponent()->m_OnEndOverlap.RemoveAll(InvenWidget);
	
	_Interactor->GetInteractionComponent()->m_OnEndOverlap.AddUObject(InvenWidget, &UC_InventoryWidget::CloseUpgradeWidget);
	
	_Interactor->GetInteractionComponent()->m_OnEndOverlap.AddUObject(_Interactor->GetInteractionComponent(), &UC_InteractionComponent::ClearCurrentInteraction, nullptr);
	// TODO : CancelInteract 구현 보고 없애야 할 수 있음.
	return true;
}

void UC_UpgradeWIdgetStrategy::CancleInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	Super::CancleInteraction(_Interactor, _TargetActor);
}

void UC_UpgradeWIdgetStrategy::CompleteInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	Super::CompleteInteraction(_Interactor, _TargetActor);
}
