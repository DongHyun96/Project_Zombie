// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GridItemSlotWidget.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "DragDropOperation/C_DragDropOperation.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "C_InventoryWidget.h"
#include "C_InventoryGridWidget.h"
#include "DivideWIdget/C_DivideItemWidget.h"

bool UC_GridItemSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
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

    // 나 자신에게 드롭하거나 드롭 대상 슬롯이 타인에 의해 잠겨있다면 요청 취소
    if ((FromInvenComp == ToInvenComp && FromSlot == ToSlot) || AssociatedInvenComp->GetInventoryItems()[ToSlot].LockedByPlayerID != INDEX_NONE)
    {
        pPlayer->Server_RequestUnlockSlot(FromInvenComp, FromSlot);
        return true;
    }

    // Ctrl + 클릭 수량 분할 로직 (Grid 전용)
    if (InDragDropEvent.IsControlDown() && FromInvenComp->GetItemAt(FromSlot).CurCount > 1)
    {
        FInventoryEntry FromEntry = FromInvenComp->GetItemAt(FromSlot);
        FInventoryEntry ToEntry = ToInvenComp->GetItemAt(ToSlot);

        UC_ItemManager* ItemManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_ItemManager>();
        if (!ItemManager) return true;

        FName TargetRowName = (ToEntry.ItemRowName == NAME_None) ? FromEntry.ItemRowName : ToEntry.ItemRowName;
        const FItemData* ToItemData = ItemManager->GetItemData<FItemData>(EItemTableType::General, TargetRowName);

        if (!ToItemData)
        {
            pPlayer->Server_RequestUnlockSlot(FromInvenComp, FromSlot);
            return true;
        }

        int32 ToItemMaxCount = ToItemData->MaxCount;

        if ((ToEntry.ItemRowName != NAME_None && FromEntry.ItemRowName != ToEntry.ItemRowName) || ToEntry.CurCount >= ToItemMaxCount)
        {
            pPlayer->Server_RequestUnlockSlot(FromInvenComp, FromSlot);
            return true;
        }

        pPlayer->Server_RequestLockSlot(ToInvenComp, ToSlot);

        if (ParentGrid)
        {
            UC_InventoryWidget* ParentInvenWidget = ParentGrid->GetParentInventoryWidget();
            if (ParentInvenWidget)
            {
                UC_DivideItemWidget* DivideWidget = ParentInvenWidget->GetDivideItemWidget();
                if (DivideWidget)
                {
                    DivideWidget->SetTargetWidget(this);
                    ParentInvenWidget->ShowDivideEntryWidget();
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("DivideWidget is Null!"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("ParentInvenWidget is Null!"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("ParentGrid is Null!"));
        }

        return true;
    }

    pPlayer->Server_RequestMoveItem(FromInvenComp, FromSlot, ToInvenComp, ToSlot);
    return true;
}