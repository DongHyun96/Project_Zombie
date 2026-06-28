// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/C_ItemSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
void UC_ItemSlotWidget::UpdateSlot(const FInventoryEntry& ItemData, const FItemData* CoreData)
{
    if (CoreData->IconTexture.IsValid())
    {
        ItemSlot->SetBrushFromTexture(CoreData->IconTexture.Get());
    }
    else
    {
        // 비동기 로드를 하거나, 인벤토리 아이콘 특성상 용량이 작으므로 동기 로드 처리
        UTexture2D* LoadedTexture = CoreData->IconTexture.LoadSynchronous();
        if (LoadedTexture)
        {
            ItemSlot->SetBrushFromTexture(LoadedTexture);
        }
    }
    SetVisibility(ESlateVisibility::Visible);
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

}
