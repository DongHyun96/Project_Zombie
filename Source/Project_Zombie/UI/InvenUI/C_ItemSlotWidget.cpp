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
#include "DivideWIdget/C_DivideItemWidget.h"
#include "GameFramework/PlayerState.h"
#include "Utility/C_Util.h"

void UC_ItemSlotWidget::UpdateSlot(const FInventoryEntry& ItemData, const FItemData* CoreData)
{
    if (ItemData.ItemRowName == NAME_None )
    {
        
        ItemIcon->SetBrushFromTexture(nullptr);
        
        ItemIconSetVisibility(ESlateVisibility::Collapsed);
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
        //if (ItemData.LockedByPlayerID != INDEX_NONE) ItemIconSetOpacity(.5);
        //else ItemIconSetOpacity(1.0);
        ItemIconSetVisibility(ESlateVisibility::Visible);
    }
    
    if (ItemData.LockedByPlayerID != INDEX_NONE) ItemIconSetOpacity(.5);
    else 
        ItemIconSetOpacity(1.0);
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
    //if (FromInvenComp->GetInventoryItems()[DragOperation->GetSlotIndex()].LockedByPlayerID == INDEX_NONE) 
    //    ItemIconSetOpacity(1.f);
    
    // 나 자신에게 드롭하거나 드롭된 슬롯이 잠겨 있다면 return
    if (FromInvenComp == ToInvenComp && FromSlot == ToSlot || AssociatedInvenComp->GetInventoryItems()[ToSlot].LockedByPlayerID != INDEX_NONE)
    {
        pPlayer->Server_RequestUnlockSlot(FromInvenComp, FromSlot);
        return true;
    }
    
    if (InDragDropEvent.IsControlDown() && FromInvenComp->GetItemAt(FromSlot).CurCount > 1)
    {
        FInventoryEntry FromEntry = FromInvenComp->GetItemAt(FromSlot);
        FInventoryEntry ToEntry = ToInvenComp->GetItemAt(ToSlot);
        
        UC_ItemManager* ItemManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_ItemManager>();
        if (!ItemManager) return true;
        
        // [중요] 드롭할 대상 슬롯이 비어있다면 원본 아이템의 정보를, 채워져 있다면 타겟 아이템의 정보를 기준으로 삼습니다.
        FName TargetRowName = (ToEntry.ItemRowName == NAME_None) ? FromEntry.ItemRowName : ToEntry.ItemRowName;
        const FItemData* curItemData = ItemManager->GetItemData(TargetRowName);
        
        // [필수] 여기서 널 체크를 해서 안전하게 반환 처리를 해줍니다.
        if (!curItemData) 
        {
            pPlayer->Server_RequestUnlockSlot(FromInvenComp, FromSlot);
            return true;
        }
        
        // 이제 curItemData가 nullptr가 아님이 확실하므로 안전하게 호출 가능합니다!
        int32 MaxCount = curItemData->MaxCount;
        
        // 현재 나의 아이템 슬롯과 드롭된 아이템이 다른 종류거나, 슬롯 수량이 이미 꽉 찼다면 분할창을 열지 않고 종료
        if ((ToEntry.ItemRowName != NAME_None && FromEntry.ItemRowName != ToEntry.ItemRowName)
            || ToEntry.CurCount >= MaxCount)
        {
            pPlayer->Server_RequestUnlockSlot(FromInvenComp, FromSlot);
            return true;
        }
        
        pPlayer->Server_RequestLockSlot(ToInvenComp, ToSlot);

        // 안전하게 검사 후 실행 TODO : NativeOnDrop 코드 정리 한번 하기.
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
        Owner->Server_CancelDragItemSlot(DragOp->GetSlotIndex(), DragOp->GetSourceComponent());
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
    BackGroundImage->SetOpacity(InOpacity);
    ItemIcon->SetOpacity(InOpacity);
    ItemCountText->SetOpacity(InOpacity);
}

void UC_ItemSlotWidget::ItemIconSetVisibility(ESlateVisibility InVisibility)
{
    //BackGroundImage->SetOpacity(InVisibility);
    ItemIcon->SetVisibility(InVisibility);
    ItemCountText->SetVisibility(InVisibility);
}
