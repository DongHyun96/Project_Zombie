// Fill out your copyright notice in the Description page of Project Settings.


#include "C_UIManager.h"

#include "Blueprint/UserWidget.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "Utility/C_Util.h"

void AC_UIManager::BeginPlay()
{
	Super::BeginPlay();

	if (!m_MainHUDClass)
	{
		UC_Util::Print("From AC_UIManager::BeginPlay : MainHUDClass Subclass nullptr", FColor::Red, 5.f);
		return;
	}

	// 이거 명확한 HUD(UUserWidget) 상위 부모 클래스가 있다하면 해당 Type으로 Casting 시도할 것
	m_MainHUDWidget = Cast<UC_GameMainHUD>(CreateWidget(GetOwningPlayerController(), m_MainHUDClass));
	
	if (!m_MainHUDWidget)
	{
		UC_Util::Print("From AC_UIManager::BeginPlay : MainHUDWidget creation failed", FColor::Red, 5.f);
		return;
	}

	m_MainHUDWidget->AddToViewport();

	if (!m_InventoryWidgetClass)
	{
		UC_Util::Print("From AC_UIManager::BeginPlay : InventoryWidget Subclass nullptr", FColor::Red, 5.f);
		return;
	}

	// 이거 명확한 HUD(UUserWidget) 상위 부모 클래스가 있다하면 해당 Type으로 Casting 시도할 것
	m_InventoryWidget = Cast<UC_InventoryWidget>(CreateWidget(GetOwningPlayerController(), m_InventoryWidgetClass));

	if (!m_InventoryWidget)
	{
		UC_Util::Print("From AC_UIManager::BeginPlay : InventoryWidget creation failed", FColor::Red, 5.f);
		return;
	}

	m_InventoryWidget->AddToViewport();
	m_InventoryWidget->SetVisibility(ESlateVisibility::Visible);
}
