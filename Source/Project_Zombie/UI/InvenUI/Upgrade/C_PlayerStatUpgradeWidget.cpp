// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/C_PlayerStatUpgradeWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_PlayerStatComponent.h"
#include "Actor/Components/InteractionComponent/C_InteractionComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Item/Interact/StatUpgrade/C_StatUpgradeStation.h"
#include "ItemDetails/C_ItemStatRowWidget.h"
#include "ItemDetails/C_ItemStatsWidget.h"
#include "PlayerStat/C_PlayerStatRowWidget.h"
#include "PlayerStat/C_PlayerStatsWidget.h"
#include "PlayerStat/C_PlayerUpMattersWidget.h"

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
	
	PlayerUpMattersWidget->UpdateWidget(m_UsePlayer); // TODO : 강화 테이블을 만들어야 넣어 줄 수 있을 듯? 
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
	
	if (!m_UsePlayer->GetInteractionComponent()) return;
	
	UC_InteractionComponent* InteractionComp = m_UsePlayer->GetInteractionComponent();
	
	if (!InteractionComp) return;
	
	AActor* actor = InteractionComp->GetCurrentInteractionTarget();
	
	if (!actor) return;
	
	AC_StatUpgradeStation* Base = Cast<AC_StatUpgradeStation>(m_UsePlayer->GetInteractionComponent()->GetCurrentInteractionTarget());

	if (!Base) return;
	
	if (SelectedStatName.IsNone()) return;
	
	//if (!m_TargetEntry->HasEquipmentData()) return;
	//
	//// 이미 최대 Grade면 서버에 요청을 보내지 않게해서 패킷 낭비를 막음.
	//UC_Util::Print(static_cast<int32>(m_TargetStat));
	//if (m_TargetEntry->GetEquipmentData()->GetStatGrade(m_TargetStat) >= MAX_GRADE) return;
	
	bIsUpgrading = true;
	
	UC_PlayerStatComponent* PlayerStatComp = Cast<UC_PlayerStatComponent>(m_UsePlayer->GetStatComponent());
	if (!PlayerStatComp) return;
	
	PlayerStatComp->Server_RequestStatUpgrade(Base, SelectedStatName);
}

void UC_PlayerStatUpgradeWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	PlayerStatsWidget->SetParentWidget(this);
	PlayerUpMattersWidget->SetParentWidget(this);
}
