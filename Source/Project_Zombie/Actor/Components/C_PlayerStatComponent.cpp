// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerStatComponent.h"

#include "GlobalData.h"
#include "Actor/Character/C_BasicCharacter.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Controller/C_BasicPlayerController.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Item/Interact/ItemUpgrade/C_ItemUpgradeStation.h"
#include "Item/Interact/StatUpgrade/C_StatUpgradeStation.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/InvenUI/Upgrade/C_PlayerStatUpgradeWidget.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/MainHUD/PlayerStatHUD/C_PlayerStatWidget.h"


UC_PlayerStatComponent::UC_PlayerStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UC_PlayerStatComponent::BeginPlay()
{
	Super::BeginPlay();

	m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
	
	if (m_OwnerCharacter->IsLocallyControlled())
	{
		AC_UIManager* UIManager = UI_MANAGER(GetWorld());
		if (!UIManager) return;

		UC_GameMainHUD* MainHUD = UIManager->GetMainHUDWidget();
		if (!MainHUD) return;

		UC_PlayerStatWidget* StatWidget = MainHUD->GetPlayerStatWidget();
		if (StatWidget)
			this->OnCurHPUpdatedDelegate.AddUObject(StatWidget, &UC_PlayerStatWidget::UpdateHPBarRatio);
	}
	// TODO : 이거 다른 사람 Stat을 표기하려면, 여기에 이런식으로 처리를 해주면 됨 (다른 팀원 체력 확인은 해야할 듯)
	else OnCurHPUpdatedDelegate.AddUObject(m_OwnerPlayer, &AC_BasicCharacter::UpdatePlayerHPOnAboveHeadTest);
	
}

void UC_PlayerStatComponent::Server_RequestStatUpgrade(AC_StatUpgradeStation* InInteractableActor,
	const FName& UpStatName)
{
	AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(m_OwnerPlayer->GetController());
	
	if (!PC) return;
	
	if (PC->GetIsUpgradingPlayerStat())
	{
		PC->Client_FinishPlayerStatUpgrade();
		return;
	}
	PC->SetIsUpgradingPlayerStat(true);
	
	InInteractableActor->RequestStatUpgrade(m_OwnerPlayer, UpStatName);
}

UScriptStruct* UC_PlayerStatComponent::GetStatDataStruct() const
{
	return FPlayerStatData::StaticStruct();
}

/*
void UC_PlayerStatComponent::InitStat()
{
	// 테이블과 행 이름이 설정되어 있어야 한다
	if (!m_Table || m_RowName.IsNone()) return;

	// 모든 스탯을 다 지운다
	m_Stats.Empty();
	
	// 테이블에 기록된 데이터에 접근한다.
	FPlayerStatData* pPlayerStat = m_Table->FindRow<FPlayerStatData>(m_RowName, TEXT("PlayerStat"));

	// 데이터를 구성하고 있는 멤버들의 멤버변수명 자체를 키값으로 해서 수치를 기록한다.
	InitStatFromStruct(FPlayerStatData::StaticStruct(), pPlayerStat);

	// Player만의 추가적인 런타임 스탯 추가
	// AddStat() ...

	// 부모 함수 호출 (공용 스탯 추가)
	Super::InitStat();
}
*/
