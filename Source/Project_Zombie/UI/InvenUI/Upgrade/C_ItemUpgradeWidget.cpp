// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/C_ItemUpgradeWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "ItemDetails/C_ItemStatsWidget.h"
#include "ItemDetails/C_MattersWidget.h"
#include "UI/InvenUI/DragDropOperation/C_DragDropOperation.h"
#include "Actor/Components/InteractionComponent/C_InteractionComponent.h"
#include "Item/Interact/C_InteractableBase.h"
#include "Item/Interact/ItemUpgrade/C_ItemUpgradeStation.h"
#include "ItemDetails/C_ItemStatRowWidget.h"
#include "ItemDetails/C_SelectedStatWidget.h"
#include "Utility/C_Util.h"

void UC_ItemUpgradeWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UC_ItemUpgradeWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UC_DragDropOperation* DragOperation = Cast<UC_DragDropOperation>(InOperation);
	
	//f (DroppedItemSlotIdx == -1) return false; NativeOnDragCancelled에서 슬롯 잠금 해제 해줘야 함.
	
	DroppedItemSlotIdx = DragOperation->GetSlotIndex();
	
	m_UsePlayer->Server_RequestUnlockSlot(m_UsePlayer->GetInvenComponent(), DroppedItemSlotIdx);
	
	if (DroppedItemSlotIdx == -1) return false;
	
	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();

	if (!ItemManager) return false;
	
	const FInventoryEntry& Entry = DragOperation->GetItemEntry();
	
	const FItemData* Data = ItemManager->GetItemData<FItemData>(EItemTableType::General, Entry.ItemRowName);
	
	if (static_cast<uint8>(Data->ItemType) >= static_cast<uint8>(EItemType::GADGET) || !DragOperation->GetItemEntry().HasEquipmentData())
	{
		DroppedItemSlotIdx = -1;
		UpdateWidget();
		return false;
	}
	
	m_TargetEntry = DragOperation->GetSourceComponent()->GetSlotDataPtr(DroppedItemSlotIdx);
	
	UpdateWidget();
	
	//if (DroppedItemSlotIdx == -1) return false;
	
	return true;
}

void UC_ItemUpgradeWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ItemStats->SetParentWidget(this);
	
	Matters->SetParentWidget(this);
}

void UC_ItemUpgradeWidget::UpdateWidget()
{	

	if (DroppedItemSlotIdx == -1 || !m_UsePlayer)
	{
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		// TODO : 여기 들어오면 위젯들 초기화 해주기.
		ItemName->SetText(FText());
		ItemDesc->SetText(FText());
		ItemStats->UpdateWidget(nullptr);
		return;
	}
	// TODO : 멤버 변수로 가지고 있는 포인터들 일일이 다 검사 해주어야 하나?
	
	const FInventoryEntry& Entry = m_UsePlayer->GetInvenComponent()->GetItemAt(DroppedItemSlotIdx);
	
	if (Entry.IsEmpty() || !Entry.HasEquipmentData()) return;

	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();

    if (!ItemManager) return;
	
	const FItemData* Data = ItemManager->GetItemData<FItemData>(EItemTableType::General, Entry.ItemRowName);

	ItemName->SetText(Data->ItemName);
	
	ItemDesc->SetText(Data->ItemDescription);
	
	ItemIcon->SetBrushFromTexture(Data->IconTexture.Get());

	ItemIcon->SetVisibility(ESlateVisibility::Visible);
	
	ItemDesc->SetText(Data->ItemDescription);

	// TODO : 아이템 동적 데이터 가져와서 보여주기
	const FEquipmentCustomData* EquipCustomData = Entry.CustomData.GetPtr<FEquipmentCustomData>();
	
	const FWeaponData* WeaponData = ItemManager->GetWeaponData(Entry.ItemRowName);;
	
	//const float CurValue = WeaponData->
	
	ItemStats->UpdateWidget(EquipCustomData);
	
	// TODO : 강화에 필요한 재료 보여주기.
	// TODO : 강화하면 재료 차감하기.
	
	
	Matters->UpdateWidget(Entry); // TODO : 강화 테이블을 만들어야 넣어 줄 수 있을 듯? 
}

void UC_ItemUpgradeWidget::InitWidget()
{
	ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
	// TODO : 여기 들어오면 위젯들 초기화 해주기.
	ItemName->SetText(FText());
	ItemDesc->SetText(FText());
	ItemStats->UpdateWidget(nullptr);
	
	DroppedItemSlotIdx = -1;
	m_UsePlayer = nullptr;
	m_TargetStat = EUpgradableStats::None;
	bIsUpgrading = false;	
	hasRequiredItems = false;
	m_TargetEntry = nullptr;
}

void UC_ItemUpgradeWidget::ShowSelectedStatRow(const float& CurStatValue, const float& NextStatValue)
{
	const FInventoryEntry& Entry = m_UsePlayer->GetInvenComponent()->GetItemAt(DroppedItemSlotIdx);
	
	const FEquipmentCustomData* EquipData = Entry.GetEquipmentData();
	
	if (!EquipData)
	{
		SelectedStatRow->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	if (!ItemManager) 
	{
		SelectedStatRow->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	const FWeaponData* Data = ItemManager->GetWeaponData(Entry.ItemRowName);
	
	if (!Data)
	{
		SelectedStatRow->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	
	
	//SelectedStatRow->UpdateWidget(m_TargetStat, EquipData->GetStatGrade(m_TargetStat), Data->);
}

void UC_ItemUpgradeWidget::RequestItemUpgrade()
{
	if (bIsUpgrading) return;
	
	UC_Util::Print(hasRequiredItems);
	
	if (!hasRequiredItems) return; 
	
	if (!m_UsePlayer) return;
	
	if (!m_UsePlayer->GetInteractionComponent()) return;
	
	UC_InteractionComponent* InteractionComp = m_UsePlayer->GetInteractionComponent();
	
	if (!InteractionComp) return;
	
	AActor* actor = InteractionComp->GetCurrentInteractionTarget();
	
	if (!actor) return;
	
	AC_ItemUpgradeStation* Base = Cast<AC_ItemUpgradeStation>(m_UsePlayer->GetInteractionComponent()->GetCurrentInteractionTarget());

	if (!Base) return;
	
	if (!m_TargetEntry) return;
	
	if (!m_TargetEntry->HasEquipmentData()) return;
	
	// 이미 최대 Grade면 서버에 요청을 보내지 않게해서 패킷 낭비를 막음.
	UC_Util::Print(static_cast<int32>(m_TargetStat));
	if (m_TargetEntry->GetEquipmentData()->GetStatGrade(m_TargetStat) >= MAX_GRADE) return;
	
	bIsUpgrading = true;
	
	m_UsePlayer->Server_RequestItemUpgrade(Base, DroppedItemSlotIdx, m_TargetStat);
}

void UC_ItemUpgradeWidget::BindingUpdateWidget(UC_InvenComponent* InInvenComp)
{
	InInvenComp->OnInventorySlotChanged.AddDynamic(this, &UC_ItemUpgradeWidget::HandleItemStatUpgraded);
}

void UC_ItemUpgradeWidget::HandleItemStatUpgraded(int32 SlotIdx, const FInventoryEntry& ItemData)
{
	if (SlotIdx != DroppedItemSlotIdx) return;
	
	const FEquipmentCustomData* EquipCustomData = ItemData.CustomData.GetPtr<FEquipmentCustomData>();
	
	ItemStats->UpdateWidget(EquipCustomData);
}

void UC_ItemUpgradeWidget::SetTargetStat(EUpgradableStats InTargetStat)
{
	m_TargetStat = InTargetStat;
	
	if (m_TargetStat == EUpgradableStats::None) return;
	
	// TODO(상연) : 원래 이런건 여기서 구현하는게 맞나? 상호작용이 일어난 StatRowWidget쪽에서 했어야 하는건 아닐까?
	
	TArray<UC_ItemStatRowWidget*> StatRowArr = ItemStats->GetItemStatRows();
	
	for (int i = 0; i < StatRowArr.Num(); ++i)
	{
		if (StatRowArr[i]->GetTargetStat() == InTargetStat)
			StatRowArr[i]->GetSelectedRow()->SetVisibility(ESlateVisibility::Visible);
		else
			StatRowArr[i]->GetSelectedRow()->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	Matters->UpdateWidget(*m_TargetEntry);
}
