// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/Upgrade/C_ItemUpgradeWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "ItemDetails/C_ItemStatsWidget.h"
#include "ItemDetails/C_MattersWidget.h"
#include "Serialization/MappedName.h"
#include "UI/InvenUI/DragDropOperation/C_DragDropOperation.h"

void UC_ItemUpgradeWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UC_ItemUpgradeWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UC_DragDropOperation* DragOperation = Cast<UC_DragDropOperation>(InOperation);
	
	// TODO : 기존에 드랍된 아이템 잠금 풀기.
	m_UsePlayer->Server_RequestUnlockSlot(m_UsePlayer->GetInvenComponent(), DroppedItemSlotIdx);
	
	DroppedItemSlotIdx = DragOperation->GetSlotIndex();
	
	UpdateWidget();
	
	if (DroppedItemSlotIdx == -1) return false;
	
	
	return true;
}

void UC_ItemUpgradeWidget::UpdateWidget()
{	
	ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
	if (DroppedItemSlotIdx == -1 || !m_UsePlayer)
	{
		// TODO : 여기 들어오면 위젯들 초기화 해주기.
		ItemName->SetText(FText());
		ItemDesc->SetText(FText());
		ItemStats->UpdateWidget(nullptr);
		return;
	}
	// TODO : 멤버 변수로 가지고 있는 포인터들 일일이 다 검사 해주어야 하나?
	
	const FInventoryEntry& Entry = m_UsePlayer->GetInvenComponent()->GetItemAt(DroppedItemSlotIdx);
	
	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
    if (!ItemManager) return;
	
	const FItemData* Data = ItemManager->GetItemData<FItemData>(EItemTableType::General, Entry.ItemRowName);

	ItemName->SetText(Data->ItemName);
	
	ItemIcon->SetBrushFromTexture(Data->IconTexture.Get());
	ItemIcon->SetVisibility(ESlateVisibility::Visible);
	
	ItemDesc->SetText(Data->ItemDescription);
	
	// TODO : 아이템 동적 데이터 가져와서 보여주기
	const FEquipmentCustomData* EquipCustomData = Entry.CustomData.GetPtr<FEquipmentCustomData>();
	
	ItemStats->UpdateWidget(EquipCustomData);
	
	// TODO : 강화에 필요한 재료 보여주기.
	//Matters->UpdateWidget(); // TODO : 강화 테이블을 만들어야 넣어 줄 수 있을 듯?
}

void UC_ItemUpgradeWidget::RequestItemUpgrade()
{
}
