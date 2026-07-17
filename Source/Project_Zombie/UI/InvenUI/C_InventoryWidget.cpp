// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/C_InventoryWidget.h"

#include "C_InventoryGridWidget.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "DivideWIdget/C_DivideItemWidget.h"
#include "DragDropOperation/C_DragDropOperation.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "Utility/C_Util.h"

void UC_InventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
	
	UC_InvenComponent* PlayerInvenComponent = Player->GetInvenComponent();

	PlayerGridWidget ->SetParentWidget(this);
	StorageGridWidget->SetParentWidget(this);
	
	PlayerGridWidget->SetInvenComponent(PlayerInvenComponent);
	
	// TODO : 오용되는 부분이 생길 수 있음.
	StorageGridWidget->SetInvenComponent(nullptr);
	
	
}

bool UC_InventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                      UDragDropOperation* InOperation)
{
	UC_DragDropOperation* DragDropOperation =  Cast<UC_DragDropOperation>(InOperation);
	
	if (!DragDropOperation) return true;
	
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
	
	if (!Player) return true;
	
	UC_InvenComponent* invenComp = Player->GetInvenComponent();

	int32 DroppedItemIdx = DragDropOperation->GetSlotIndex();
	
	const FInventoryEntry curSlotItem = invenComp->GetItemAt(DroppedItemIdx);
	
	if (InDragDropEvent.IsControlDown() && curSlotItem.CurCount > 1)
	{
		DivideItemWidget->SetTargetWidget(this);
		ShowDivideItemWidget();
		return true;
	}
	
	// TODO : 서버에게 DropItemByPlayer를 요청하는 방식으로 가야함.
	//Player->Server_RequestDropItemByPlayer();
	
	SpawnItem(DragDropOperation, curSlotItem.CurCount);
	
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UC_InventoryWidget::ShowDivideEntryWidget()
{
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
	
	if (!Player) return;
	
	FCursorItem cursorItem = Player->GetCurDraggedItem();
	
	if (!cursorItem.bIsValid) return;
	
	DivideItemWidget->SetCursorItem(cursorItem);
	
	DivideItemWidget->ShowDivideEntry();
}

void UC_InventoryWidget::ShowDivideItemWidget()
{
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
	
	if (!Player) return;
	
	FCursorItem cursorItem = Player->GetCurDraggedItem();
	
	if (!cursorItem.bIsValid) return;
	
	DivideItemWidget->SetCursorItem(cursorItem);
	
	DivideItemWidget->ShowDivideItem();
	
}

void UC_InventoryWidget::SetVisibility(ESlateVisibility InVisibility)
{
	Super::SetVisibility(InVisibility);
	
	if (InVisibility == ESlateVisibility::Visible)
	{
		if (StorageGridWidget->GetInvenComponent())
			StorageGridWidget->SetVisibility(InVisibility);
		else
			StorageGridWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
}

void UC_InventoryWidget::SpawnItem(UC_DragDropOperation* InDragDropOperation, int32 InCount)
{
	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	UC_InvenComponent* invenComp = Cast<AC_BasicPlayer>(GetOwningPlayerPawn())->GetInvenComponent();

	int32 DroppedItemIdx = InDragDropOperation->GetSlotIndex();
	
	FInventoryEntry curSlotItem = invenComp->GetInventoryItems()[DroppedItemIdx];

	UC_ItemManager* ItemMgr = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	if (curSlotItem.CurCount == InCount)
	{
		invenComp->InitInvenItemAt(DroppedItemIdx);
	}
	else if (curSlotItem.CurCount > InCount && InCount > 0)
	{
		invenComp->SetEntryCurCount(DroppedItemIdx, curSlotItem.CurCount - InCount);
	}
	else return;

	// 3. 변경된 SpawnItem 호출 (계산한 힘을 넘겨줌)
	ItemMgr->DropItemByPlayer(curSlotItem.ItemRowName,  InCount, GetOwningPlayerPawn());
	
	PlayerGridWidget->RefreshSlotAt(DroppedItemIdx, invenComp->GetInventoryItems()[DroppedItemIdx]);
}
