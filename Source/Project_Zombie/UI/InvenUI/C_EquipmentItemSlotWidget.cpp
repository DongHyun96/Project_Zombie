// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/C_EquipmentItemSlotWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "DragDropOperation/C_DragDropOperation.h"
#include "GameModeAndManager/C_ItemManager.h"

bool UC_EquipmentItemSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UC_DragDropOperation* DragOperation = Cast<UC_DragDropOperation>(InOperation);
	if (!DragOperation || !AssociatedInvenComp) return false;

	UC_InvenComponent* FromInvenComp = DragOperation->GetSourceComponent();
	UC_InvenComponent* ToInvenComp = AssociatedInvenComp;
	if (!FromInvenComp || !ToInvenComp) return false;

	int32 FromSlot = DragOperation->GetSlotIndex();
	int32 ToSlot = curSlotIdx;

	AC_BasicPlayer* pPlayer = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
	if (!pPlayer) return false;
	
	// 장비 슬롯 타입 일치 검사
	FInventoryEntry FromEntry = FromInvenComp->GetItemAt(FromSlot);
	UC_ItemManager* ItemManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_ItemManager>();
	if (ItemManager)
	{
		const FItemData* FromItemData = ItemManager->GetItemData<FItemData>(EItemTableType::General, FromEntry.ItemRowName);

		// 아이템 정보가 없거나, 이 장비 슬롯의 EWeaponSlot 조건과 맞지 않으면 드롭 거부
		if (!FromItemData || FromItemData->ItemType != AllowedItemType)
		{
			pPlayer->Server_RequestUnlockSlot(FromInvenComp, FromSlot);
			return true;
		}
	}

	// 자기 자신 슬롯 드롭 및 슬롯 잠금 여부 검사
	if ((FromInvenComp == ToInvenComp && FromSlot == ToSlot) || AssociatedInvenComp->GetInventoryItems()[ToSlot].LockedByPlayerID != INDEX_NONE)
	{
		pPlayer->Server_RequestUnlockSlot(FromInvenComp, FromSlot);
		return true;
	}

	// 조건 통과 시 서버에 아이템 이동/장착 요청
	pPlayer->Server_RequestMoveItem(FromInvenComp, FromSlot, ToInvenComp, ToSlot);
	return true;
}