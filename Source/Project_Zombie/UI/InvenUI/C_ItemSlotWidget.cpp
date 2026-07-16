// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/C_ItemSlotWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "DragDropOperation/C_DragDropOperation.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "GameModeAndManager/C_UIManager.h"
#include "C_InventoryWidget.h"
#include "C_InventoryGridWidget.h"
#include "Components/Border.h"
#include "GameFramework/PlayerState.h"
#include "Utility/C_Util.h"

void UC_ItemSlotWidget::UpdateSlot(const FInventoryEntry& ItemData, const FItemData* CoreData)
{
    if (ItemData.ItemRowName == NAME_None)
    {
        ItemIcon->SetBrushFromTexture(nullptr);
        ItemIconSetVisibility(ESlateVisibility::Collapsed);
        ItemIconSetOpacity(1.0);
    }
    else
    {
        if (CoreData->IconTexture.IsValid())
        {
            ItemIcon->SetBrushFromTexture(CoreData->IconTexture.Get());
            SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            // 비동기 로드를 하거나, 인벤토리 아이콘 특성상 용량이 작으므로 동기 로드 처리
            UTexture2D* LoadedTexture = CoreData->IconTexture.LoadSynchronous();
            if (LoadedTexture)
            {
                ItemIcon->SetBrushFromTexture(LoadedTexture);
                SetVisibility(ESlateVisibility::Visible);
            }
        }
        ItemCountText->SetText(FText::AsNumber(ItemData.CurCount));
        
        // 드래그 중이면 오퍼시티를 .5로 변경 
        if (ItemData.LockedByPlayerID != INDEX_NONE) ItemIconSetOpacity(.5);
        else ItemIconSetOpacity(1.0);
        ItemIconSetVisibility(ESlateVisibility::Visible);
    }
}

FReply UC_ItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        FEventReply RePlyResult =
            UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
    
        return RePlyResult.NativeReply;
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UC_ItemSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    if (!AssociatedInvenComp) return;

    AC_BasicPlayer* Owner = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
    
    if (!Owner) return;
    
    // 서버에 아이템 드래그 요청
    Owner->Server_RequestDragItemSlot(curSlotIdx, AssociatedInvenComp); // 창고에 있는 아이템의 PlayerID를 건드려야함.
    
    const TArray<FInventoryEntry>& ItemArr = AssociatedInvenComp->GetInventoryItems();
    
    if (!ItemArr.IsValidIndex(curSlotIdx)) return;
    
    FInventoryEntry entry = ItemArr[curSlotIdx];

    AC_BasicPlayer* pPlayer = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
    
    // Player의 curDraggedItem 세팅
    if (!pPlayer->SetCurDraggedItem(entry, AssociatedInvenComp, curSlotIdx)) return;
    
    //if (entry.ItemRowName == NAME_None) return;
    
    UC_DragDropOperation* DragOperation = NewObject<UC_DragDropOperation>();
    
    // DragVisual 생성
    InitDragVisual(DragOperation);
    
    DragOperation->SetItemEntry(entry);
    DragOperation->SetSourceComponent(AssociatedInvenComp);
    DragOperation->SetSlotIndex(curSlotIdx);
    
    OutOperation = DragOperation;
}

bool UC_ItemSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    UC_DragDropOperation* DragOperation = Cast<UC_DragDropOperation>(InOperation);
    
    if (!DragOperation) return false;
    
    if (!AssociatedInvenComp) return false;
    
    UC_InvenComponent* FromInvenComp = DragOperation->GetSourceComponent();
    UC_InvenComponent* ToInvenComp = AssociatedInvenComp;;
    
    if (!FromInvenComp || !ToInvenComp) return false;
    
    int32 FromSlot = DragOperation->GetSlotIndex();
    int32 ToSlot = curSlotIdx;

    AC_BasicPlayer* pPlayer = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
    
    if (!pPlayer) return false;
    
    // 드래그시 반감된 오파시티 복구(제자리에 드롭할 때 .5가 유지되서 넣은 코드)
    if (FromInvenComp->GetInventoryItems()[DragOperation->GetSlotIndex()].LockedByPlayerID == INDEX_NONE) 
        ItemIconSetOpacity(1.f);
    
    
    
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD()))
        {
            if (UC_InventoryWidget* InveWidget = UIManager->GetInventoryWidget())
                InveWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
    }
    
    // 드롭된 슬롯이 잠겨 있다면 드롭 실패. TODO : 실패 처리가 이렇게 추가되면 위에 드래그시 반감된 오파시티 복구 코드는 삭제해도 되나?
    if (AssociatedInvenComp->GetInventoryItems()[ToSlot].LockedByPlayerID != INDEX_NONE)
    {
        // 플레이어의 CurDraggedItem 초기화
        pPlayer->ClearCurDraggedItem();
        return true;
    }
    
    if (InDragDropEvent.IsControlDown())
    {
        ParentGrid->GetParentWidget()->ShowDivideWidget();
        return true;
    }
    
    pPlayer->Server_RequestMoveItem(FromInvenComp, FromSlot, ToInvenComp, ToSlot);
    
    // TODO : FFastArraySerializer 전환 하면 여기 바꾸기
    //ToInvenComp->Server_RequestMoveItem(FromInvenComp, FromSlot, ToInvenComp, ToSlot);
    
    return true;
}

void UC_ItemSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
    UC_DragDropOperation* DragOp = Cast<UC_DragDropOperation>(InOperation);
    
    AC_BasicPlayer* Owner = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
    
    // 드래그시 변경된 오파시티 초기화
    ItemIconSetOpacity(1.f);
    
    if(DragOp && Owner)
    {
        // 서버에 잠금 해제 요청
        Owner->Server_CancelDragItemSlot_Implementation(DragOp->GetSlotIndex(), AssociatedInvenComp);
    }
}


void UC_ItemSlotWidget::InitDragVisual(UC_DragDropOperation* InDragDropOp)
{
    UBorder* Border = NewObject<UBorder>();
    FLinearColor BorderColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.1f); // (R, G, B, A)
    Border->SetBrushColor(BorderColor);
    UImage* DragVisual = NewObject<UImage>(this);
    
    DragVisual->SetBrush(ItemIcon->Brush);
    DragVisual->Brush.ImageSize = FVector2D(64.f, 64.f);
    Border->SetContent(DragVisual);
    
    InDragDropOp->DefaultDragVisual = Border;
    
    InDragDropOp->Pivot = EDragPivot::CenterCenter;
}

void UC_ItemSlotWidget::ItemIconSetOpacity(float InOpacity)
{
    ItemIcon->SetOpacity(InOpacity);
    ItemCountText->SetOpacity(InOpacity);
}

void UC_ItemSlotWidget::ItemIconSetVisibility(ESlateVisibility InVisibility)
{
    ItemIcon->SetVisibility(InVisibility);
    ItemCountText->SetVisibility(InVisibility);
}
