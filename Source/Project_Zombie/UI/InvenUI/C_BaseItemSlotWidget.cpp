// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/C_BaseItemSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "DragDropOperation/C_DragDropOperation.h"
#include "Utility/C_Util.h"

void UC_BaseItemSlotWidget::UpdateSlot(const FInventoryEntry& ItemData, const FItemData* CoreData)
{
    if (ItemData.ItemRowName == NAME_None)
    {
        ItemIcon->SetBrushFromTexture(nullptr);
        ItemIconSetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        if (CoreData && CoreData->IconTexture.IsValid())
        {
            ItemIcon->SetBrushFromTexture(CoreData->IconTexture.Get());
            SetVisibility(ESlateVisibility::Visible);
        }
        else if (CoreData)
        {
            UTexture2D* LoadedTexture = CoreData->IconTexture.LoadSynchronous();
            if (LoadedTexture)
            {
                ItemIcon->SetBrushFromTexture(LoadedTexture);
                SetVisibility(ESlateVisibility::Visible);
            }
        }
        ItemCountText->SetText(FText::AsNumber(ItemData.CurCount));
        ItemIconSetVisibility(ESlateVisibility::Visible);
    }

    if (ItemData.LockedByPlayerID != INDEX_NONE)
    {
        ItemIconSetOpacity(0.5f);
    }
    else
    {
        ItemIconSetOpacity(1.0f);
    }
    
    UC_Util::Print(curSlotIdx);
}

FReply UC_BaseItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        FEventReply RePlyResult = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
        return RePlyResult.NativeReply;
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UC_BaseItemSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    if (!AssociatedInvenComp) return;

    AC_BasicPlayer* Owner = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
    if (!Owner) return;

    Owner->Server_RequestDragItemSlot(curSlotIdx, AssociatedInvenComp);

    const TArray<FInventoryEntry>& ItemArr = AssociatedInvenComp->GetInventoryItems();
    if (!ItemArr.IsValidIndex(curSlotIdx)) return;

    FInventoryEntry entry = ItemArr[curSlotIdx];
    if (!Owner->SetCurDraggedItem(entry, AssociatedInvenComp, curSlotIdx)) return;

    UC_DragDropOperation* DragOperation = NewObject<UC_DragDropOperation>();
    InitDragVisual(DragOperation);

    DragOperation->SetItemEntry(entry);
    DragOperation->SetSourceComponent(AssociatedInvenComp);
    DragOperation->SetSlotIndex(curSlotIdx);

    OutOperation = DragOperation;
}

void UC_BaseItemSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

    UC_DragDropOperation* DragOp = Cast<UC_DragDropOperation>(InOperation);
    AC_BasicPlayer* Owner = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());

    ItemIconSetOpacity(1.0f);

    if (DragOp && Owner)
    {
        Owner->Server_CancelDragItemSlot(DragOp->GetSlotIndex(), DragOp->GetSourceComponent());
    }
}

void UC_BaseItemSlotWidget::InitDragVisual(UC_DragDropOperation* InDragDropOp)
{
    UBorder* Border = NewObject<UBorder>(this);
    FLinearColor BorderColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.1f);
    Border->SetBrushColor(BorderColor);

    UImage* DragVisual = NewObject<UImage>(this);
    DragVisual->SetBrush(ItemIcon->GetBrush());
    DragVisual->SetBrushSize(FVector2D(64.f, 64.f));
    DragVisual->Brush.ImageSize = FVector2D(64.f, 64.f);
    Border->SetContent(DragVisual);

    InDragDropOp->DefaultDragVisual = Border;
    InDragDropOp->Pivot = EDragPivot::CenterCenter;
}

void UC_BaseItemSlotWidget::ItemIconSetOpacity(float InOpacity)
{
    if (BackGroundImage) BackGroundImage->SetOpacity(InOpacity);
    if (ItemIcon) ItemIcon->SetOpacity(InOpacity);
    if (ItemCountText) ItemCountText->SetOpacity(InOpacity);
}

void UC_BaseItemSlotWidget::ItemIconSetVisibility(ESlateVisibility InVisibility)
{
    if (ItemIcon) ItemIcon->SetVisibility(InVisibility);
    if (ItemCountText) ItemCountText->SetVisibility(InVisibility);
}