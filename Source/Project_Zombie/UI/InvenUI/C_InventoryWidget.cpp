// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/C_InventoryWidget.h"

#include "C_InventoryGridWidget.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Controller/C_BasicPlayerController.h"
#include "DivideWIdget/C_DivideItemWidget.h"
#include "DragDropOperation/C_DragDropOperation.h"
#include "Equipment/C_EquipmentWidget.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "Upgrade/C_ItemUpgradeWidget.h"
#include "Upgrade/C_PlayerStatUpgradeWidget.h"
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
	UC_DragDropOperation* DragDropOperation = Cast<UC_DragDropOperation>(InOperation);
	if (!DragDropOperation) return true;
    
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
	if (!Player) return true;
    
	// 현재 들고 있는 드래그 정보 가져오기
	FCursorItem DraggedItem = Player->GetCurDraggedItem();
	if (!DraggedItem.bIsValid || !DraggedItem.SourceInvenComp) return true;

	int32 DroppedItemIdx = DragDropOperation->GetSlotIndex();
    
	// 안전을 위해 컴포넌트에서 실시간 개수 재확인 (혹은 DraggedItem 내 entry 개수 활용)
	const FInventoryEntry CurSlotItem = DraggedItem.SourceInvenComp->GetItemAt(DroppedItemIdx);
    
	// [경우 A] Ctrl 키를 누른 채 빈 곳에 드롭했고 개수가 2개 이상이면 -> 분할 패널 오픈
	if (InDragDropEvent.IsControlDown() && CurSlotItem.CurCount > 1)
	{
		DivideItemWidget->SetTargetWidget(this);
		ShowDivideItemWidget(); // 여기서 유저가 수량 정하고 확인 누르면 서버 RPC 호출하게 연동 필요
		return true;
	}
    
	// [경우 B] 그냥 드롭한 경우 -> 들고 있던 슬롯 전체(CurCount)를 바닥에 던짐 (서버 RPC 호출)
	Player->Server_RequestDivideDropItem(DraggedItem.SourceInvenComp, DroppedItemIdx, CurSlotItem.CurCount);
    
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

void UC_InventoryWidget::ShowItemUpgradeWidget()
{
	UpgradeWidget->SetVisibility(ESlateVisibility::Visible);
}

void UC_InventoryWidget::CloseItemUpgradeWidget()
{
	UpgradeWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UC_InventoryWidget::ShowPlayerStatUpgradeWidget()
{
	PlayerStatUpgradeWidget->SetVisibility(ESlateVisibility::Visible);
}

void UC_InventoryWidget::ClosePlayerStatUpgradeWidget()
{
	PlayerStatUpgradeWidget->SetVisibility(ESlateVisibility::Collapsed);
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
