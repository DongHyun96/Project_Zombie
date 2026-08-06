// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/C_PlayerStatUpgradeWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_PlayerStatComponent.h"
#include "Actor/Components/InteractionComponent/C_InteractionComponent.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Controller/C_BasicPlayerController.h"
#include "Item/Interact/StatUpgrade/C_StatUpgradeStation.h"
#include "ItemDetails/C_ItemStatRowWidget.h"
#include "ItemDetails/C_ItemStatsWidget.h"
#include "PlayerStat/C_PlayerStatRowWidget.h"
#include "PlayerStat/C_PlayerStatsWidget.h"
#include "PlayerStat/C_PlayerUpMattersWidget.h"
#include "Utility/C_Util.h"

void UC_PlayerStatUpgradeWidget::UpdateWidget()
{
	if (!m_UsePlayer) return;
	
	if (!m_UsePlayer)
	{
		PlayerIcon->SetVisibility(ESlateVisibility::Collapsed);
		// TODO : 여기 들어오면 위젯들 초기화 해주기.
		PlayerName->SetText(FText());
		PlayerStatsWidget->UpdateWidget(nullptr);
		return;
	}
	// TODO : 멤버 변수로 가지고 있는 포인터들 일일이 다 검사 해주어야 하나?
	
	//const FInventoryEntry& Entry = m_UsePlayer->GetInvenComponent()->GetItemAt(DroppedItemSlotIdx);
	//
	//if (Entry.IsEmpty() || !Entry.HasEquipmentData()) return;
	//
	//UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	//
	//if (!ItemManager) return;
	
	//const FItemData* Data = ItemManager->GetItemData<FItemData>(EItemTableType::General, Entry.ItemRowName);

	// TODO : PlayerName 띄워 줄 수 있게 하기? 사실 없어도 될 것 같기도 하고.
	//PlayerName->SetText(Data->ItemName);
	
	//ItemDesc->SetText(Data->ItemDescription);
	//
	//ItemIcon->SetBrushFromTexture(Data->IconTexture.Get());

	// TODO : 이것도 사실 없어도 문제 없음.
	PlayerIcon->SetVisibility(ESlateVisibility::Visible);
	
	//ItemDesc->SetText(Data->ItemDescription);

	PlayerStatsWidget->UpdateWidget(m_UsePlayer->GetStatComponent());
	
	PlayerUpMattersWidget->UpdateWidget(m_UsePlayer);
	
	//UpdateButton(true);
}

void UC_PlayerStatUpgradeWidget::SetSelectedStatName(const FName& InSelectedStatName)
{
	SelectedStatName = InSelectedStatName;
	
	if (!SelectedStatName.IsValid()) return;
	
	// TODO(상연) : 원래 이런건 여기서 구현하는게 맞나? 상호작용이 일어난 StatRowWidget쪽에서 했어야 하는건 아닐까?
	
	TArray<UC_PlayerStatRowWidget*> StatRowArr = PlayerStatsWidget->GetItemStatRows();
	
	for (int i = 0; i < StatRowArr.Num(); ++i)
	{
		if (StatRowArr[i]->GetTargetStat() == SelectedStatName)
			StatRowArr[i]->GetSelectedRow()->SetVisibility(ESlateVisibility::Visible);
		else
			StatRowArr[i]->GetSelectedRow()->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	PlayerUpMattersWidget->UpdateWidget(m_UsePlayer);
	
	UpdateButton(true);
}

void UC_PlayerStatUpgradeWidget::InitWidget()
{
	//PlayerIcon->SetVisibility(ESlateVisibility::Collapsed);
	// TODO : 여기 들어오면 위젯들 초기화 해주기.
	PlayerName->SetText(FText());
	//PlayerStatsWidget->UpdateWidget(FInventoryEntry());
	
	m_UsePlayer = nullptr;
	SelectedStatName = FName();
	
	bIsUpgrading = false;	
	hasRequiredItems = false;
}

void UC_PlayerStatUpgradeWidget::RequestStatUpgrade()
{
	if (bIsUpgrading) return;
	
	//UC_Util::Print(hasRequiredItems);
	
	if (!hasRequiredItems) return; 
	
	if (!m_UsePlayer) return;
	
	if (Cast<AC_BasicPlayerController>(m_UsePlayer->GetController())->GetIsUpgradingPlayerStat()) return;
	
	if (!m_UsePlayer->GetInteractionComponent()) return;
	
	UC_InteractionComponent* InteractionComp = m_UsePlayer->GetInteractionComponent();
	
	if (!InteractionComp) return;
	
	AActor* actor = InteractionComp->GetCurrentInteractionTarget();
	
	if (!actor) return;
	
	AC_StatUpgradeStation* Base = Cast<AC_StatUpgradeStation>(m_UsePlayer->GetInteractionComponent()->GetCurrentInteractionTarget());

	if (!Base) return;
	
	if (SelectedStatName.IsNone()) return;
	
	UC_PlayerStatComponent* PlayerStatComp = Cast<UC_PlayerStatComponent>(m_UsePlayer->GetStatComponent());
	if (!PlayerStatComp) return;
	
	if (PlayerStatComp->GetStatGrade(SelectedStatName) >= MAX_GRADE)
	{
		SetSelectedStatName(SelectedStatName);
		UpdateButton(false);
		return;
	}
	
	bIsUpgrading = true;
	
	UpdateButton(false);
	
	PlayerStatComp->Server_RequestStatUpgrade(Base, SelectedStatName);
}

void UC_PlayerStatUpgradeWidget::UpdateButton(bool InAllow)
{
	
	/*// 1. 주요 변수 및 조건 값 계산
	const bool bIsStatSelected     = !SelectedStatName.IsNone();
	const bool bControllerUpgrading = m_UsePlayer && m_UsePlayer->GetController() 
		? Cast<AC_BasicPlayerController>(m_UsePlayer->GetController())->GetIsUpgradingPlayerStat()
		: false;

	const bool bCanClick = bIsStatSelected && !bIsUpgrading && hasRequiredItems && !bControllerUpgrading;

	// 2. 화면 출력 (TimeToDisplay = 10.f)
	UC_Util::Print(FString::Printf(TEXT("=== [Upgrade Button Conditions] Final CanClick: %s ==="), bCanClick ? TEXT("TRUE") : TEXT("FALSE")), 
				   bCanClick ? FColor::Green : FColor::Red, 10.f);

	UC_Util::Print(FString::Printf(TEXT("1. SelectedStatName: %s (IsSelected: %s)"), 
				   *SelectedStatName.ToString(), bIsStatSelected ? TEXT("OK") : TEXT("FAIL")), 
				   bIsStatSelected ? FColor::White : FColor::Orange, 10.f);

	UC_Util::Print(FString::Printf(TEXT("2. bIsUpgrading (Widget): %s (Expected: FALSE)"), 
				   bIsUpgrading ? TEXT("TRUE (FAIL)") : TEXT("FALSE (OK)")), 
				   !bIsUpgrading ? FColor::White : FColor::Orange, 10.f);

	UC_Util::Print(FString::Printf(TEXT("3. hasRequiredItems: %s"), 
				   hasRequiredItems ? TEXT("TRUE (OK)") : TEXT("FALSE (FAIL)")), 
				   hasRequiredItems ? FColor::White : FColor::Orange, 10.f);

	UC_Util::Print(FString::Printf(TEXT("4. Controller IsUpgrading: %s (Expected: FALSE)"), 
				   bControllerUpgrading ? TEXT("TRUE (FAIL)") : TEXT("FALSE (OK)")), 
				   !bControllerUpgrading ? FColor::White : FColor::Orange, 10.f);*/
	
	if (SelectedStatName.IsNone())  return UpgradeBtn->SetIsEnabled(false);
	if (bIsUpgrading)				return UpgradeBtn->SetIsEnabled(false);
	if (!hasRequiredItems)			return UpgradeBtn->SetIsEnabled(false);
	
	if (Cast<AC_BasicPlayerController>(m_UsePlayer->GetController())->GetIsUpgradingPlayerStat())
		return UpgradeBtn->SetIsEnabled(false);
	
	if (!InAllow)  return UpgradeBtn->SetIsEnabled(false);
	
	UpgradeBtn->SetIsEnabled(true);	
}

void UC_PlayerStatUpgradeWidget::BindStatEvents(UC_StatComponentBase* InStatComp)
{
	InStatComp->OnStatGradeUpdatedDelegate.AddUObject(this, &UC_PlayerStatUpgradeWidget::OnStatGradeChanged);
}

void UC_PlayerStatUpgradeWidget::OnStatGradeChanged(const FName& StatName, uint8 NewGrade)
{
	if (!m_UsePlayer && !m_UsePlayer->GetStatComponent()) return;
	
	PlayerStatsWidget->UpdateWidget(m_UsePlayer->GetStatComponent());
}

void UC_PlayerStatUpgradeWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	PlayerStatsWidget->SetParentWidget(this);
	PlayerUpMattersWidget->SetParentWidget(this);
}
