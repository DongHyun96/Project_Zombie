// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/C_ItemSlotWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "DragDropOperation/C_DragDropOperation.h"
#include "GameMode/C_ItemManager.h"
#include "GameMode/C_UIManager.h"
#include "C_InventoryWidget.h"
#include "C_InventoryGridWidget.h"
#include "Components/Border.h"
#include "Utility/C_Util.h"

void UC_ItemSlotWidget::UpdateSlot(const FInventoryEntry& ItemData, const FItemData* CoreData)
{
    if (ItemData.ItemRowName == NAME_None)
    {
        ItemSlot->SetBrushFromTexture(nullptr);
    }
    else
    {
        if (CoreData->IconTexture.IsValid())
        {
            ItemSlot->SetBrushFromTexture(CoreData->IconTexture.Get());
            SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            // 비동기 로드를 하거나, 인벤토리 아이콘 특성상 용량이 작으므로 동기 로드 처리
            UTexture2D* LoadedTexture = CoreData->IconTexture.LoadSynchronous();
            if (LoadedTexture)
            {
                ItemSlot->SetBrushFromTexture(LoadedTexture);
                SetVisibility(ESlateVisibility::Visible);
            }
        }
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
    
    const TArray<FInventoryEntry>& ItemArr = AssociatedInvenComp->GetInventoryItems();
    
    if (!ItemArr.IsValidIndex(curSlotIdx)) return;
    
    FInventoryEntry entry = ItemArr[curSlotIdx];

    if (entry.ItemRowName == NAME_None) return;
    
    UC_DragDropOperation* DragOperation = NewObject<UC_DragDropOperation>();
    
    // DragVisual 생성
    InitDragVisual(DragOperation);
    
    DragOperation->SetItemEntry(entry);
    DragOperation->SetSlotIndex(curSlotIdx);
    DragOperation->SetSourceComponent(AssociatedInvenComp);

    
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
    
    ToInvenComp->Server_RequestMoveItem(FromInvenComp, FromSlot, ToInvenComp, ToSlot);
    
    return true;
}

void UC_ItemSlotWidget::InitDragVisual(UC_DragDropOperation* InDragDropOp)
{
    UBorder* Border = NewObject<UBorder>();
    FLinearColor BorderColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.1f); // (R, G, B, A)
    Border->SetBrushColor(BorderColor);
    UImage* DragVisual = NewObject<UImage>(this);
    
    DragVisual->SetBrush(ItemSlot->Brush);
    DragVisual->Brush.ImageSize = FVector2D(64.f, 64.f);
    Border->SetContent(DragVisual);
    
    InDragDropOp->DefaultDragVisual = Border;
    
    InDragDropOp->Pivot = EDragPivot::CenterCenter;
}
