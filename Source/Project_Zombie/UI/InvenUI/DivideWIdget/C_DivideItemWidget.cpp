// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/DivideWIdget/C_DivideItemWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/InvenUI/C_ItemSlotWidget.h"
#include "Utility/C_Util.h"

void UC_DivideItemWidget::UpdateWidget()
{
	if (!CursorItem.bIsValid) return;
	
	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	if (!ItemManager) return;
	
	const FItemData* curDraggedItemData = ItemManager->GetItemData(CursorItem.Item.ItemRowName);
	
	if (!curDraggedItemData) return;
	
	int32 MaxCount = CursorItem.Item.CurCount;
	
	CalculateDivideCount(FMath::TruncToInt32(MaxCount * 0.5f));
	
	CountSlider->SetMaxValue(MaxCount);
	
	MaxItemCountText->SetText(FText::FromString(FString::FromInt(MaxCount)));
	
	ItemIcon->SetBrushFromTexture(curDraggedItemData->IconTexture.Get());
	
	UpdateDroppedCount();
	
	UpdateSlider();
	
	ItemNameTextBlock->SetText(curDraggedItemData->ItemName);
	
	DroppedCountText->SetSelectAllTextWhenFocused(true);
	
	DroppedCountText->SetKeyboardFocus();
	
	SetVisibility(ESlateVisibility::Visible);
}

void UC_DivideItemWidget::OnDivideConfirm(int32 RequestedDivideCount)
{
}

void UC_DivideItemWidget::OnDivideConfirmFroDrop(int32 RequestedDivideCount)
{
}

FReply UC_DivideItemWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	UC_Util::Print("Down!!");
	FKey PressedKey = InKeyEvent.GetKey();
	
	if (PressedKey == EKeys::Escape)
	{
		HandleOnClickButtonExitBtn();
		return FReply::Handled();
	}
	
	if (PressedKey == EKeys::Enter)
	{
		if (DivideEntryButton->GetVisibility() == ESlateVisibility::Visible)
		{
			HandleOnClickButtonDivideEntry();
			return FReply::Handled();
		}
		
		if (DivideItemButton->GetVisibility() == ESlateVisibility::Visible)
		{
			HandleOnClickButtonDivideItem();
			return FReply::Handled();
		}
	}
	
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UC_DivideItemWidget::CalculateDivideCount(int32 InCurCount)
{
	if (InCurCount <= 0)
	{
		DividedCount = 1;
		
		CountSlider->SetKeyboardFocus();
		
		// 위에서 포커스를 잠시 다른데 주고 다음 틱에 DroppedCountText가 포커스를 갖도록 해서 SelectAll 하도록 함.
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UC_DivideItemWidget::ReFocusDroppedCountText);
	}
	else if (InCurCount > CursorItem.Item.CurCount)
	{
		DividedCount = CursorItem.Item.CurCount;
	}
	else
	{
		DividedCount = InCurCount;
	}
}

void UC_DivideItemWidget::ShowDivideEntry()
{
	UpdateWidget();
	
	// 위에서 다 한번 켜주는거 아닌가? 근데 왜 ShowDivideItem하고 키면 안보이지?
	DivideEntryButton->SetVisibility(ESlateVisibility::Visible);
	
	DivideItemButton->SetVisibility(ESlateVisibility::Collapsed);
}

void UC_DivideItemWidget::ShowDivideItem()
{
	UpdateWidget();
	
	// 위에서 다 한번 켜주는거 아닌가? 근데 왜 ShowDivideEntry하고 키면 안보이지?
	DivideItemButton->SetVisibility(ESlateVisibility::Visible);
	
	DivideEntryButton->SetVisibility(ESlateVisibility::Collapsed);
}

void UC_DivideItemWidget::UpdateSlider()
{
	CountSlider->SetValue(static_cast<float>(DividedCount));
}


void UC_DivideItemWidget::UpdateDroppedCount()
{
	DroppedCountText->SetText(FText::FromString(FString::FromInt(DividedCount)));
}

void UC_DivideItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	CountSlider->OnValueChanged.AddUniqueDynamic(
		this,
		&UC_DivideItemWidget::HandleOnValueChangedCountSlider);

	DroppedCountText->OnTextChanged.AddUniqueDynamic(
		this,
		&UC_DivideItemWidget::HandleOnTextChangedDroppedCount);
	
	DivideEntryButton->OnReleased.AddUniqueDynamic(
		this,
		&UC_DivideItemWidget::HandleOnClickButtonDivideEntry);
	
	DivideItemButton->OnReleased.AddUniqueDynamic(
		this,
		&UC_DivideItemWidget::HandleOnClickButtonDivideItem);
	
	ExitButton->OnReleased.AddUniqueDynamic(
		this,
		&UC_DivideItemWidget::HandleOnClickButtonExitBtn);
}

void UC_DivideItemWidget::HandleOnValueChangedCountSlider(float InValue)
{
	DividedCount = FMath::TruncToInt32(InValue);

	UpdateSlider();
	
	UpdateDroppedCount();
}

void UC_DivideItemWidget::HandleOnTextChangedDroppedCount(const FText& InText)
{
	FString TextString = InText.ToString();
    
	CalculateDivideCount(FCString::Atoi(*TextString));
	
	UpdateSlider();
	
	UpdateDroppedCount();
}

void UC_DivideItemWidget::HandleOnClickButtonDivideEntry()
{
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
	
	if (!Player) return;
	
	UC_ItemSlotWidget* TargetSlot = Cast<UC_ItemSlotWidget>(TargetWidget);
	
	if (!TargetSlot) return;
	
	UC_InvenComponent* SrcInvenComp = CursorItem.SourceInvenComp;
	
	int32 SrcIdx = CursorItem.SourceSlotIndex;
	
	UC_InvenComponent* DstInvenComp = TargetSlot->GetAssociatedComponent();
	
	int32 DstIdx = TargetSlot->GetSlotIndex();
	
	Player->Server_RequestDivideMoveItem(SrcInvenComp, SrcIdx, 
		DstInvenComp, DstIdx, 
		DividedCount);
		
	SetVisibility(ESlateVisibility::Collapsed);
	
	// CursorItem Clear
	CursorItem.Clear();
}

void UC_DivideItemWidget::HandleOnClickButtonDivideItem()
{
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
	
	if (!Player) return;
	
	UC_InventoryWidget* TargetSlot = Cast<UC_InventoryWidget>(TargetWidget);
	
	if (!TargetSlot) return;
	
	UC_InvenComponent* SrcInvenComp = CursorItem.SourceInvenComp;
	
	if (!SrcInvenComp) return;
	
	int32 SrcIdx = CursorItem.SourceSlotIndex;
	
	Player->Server_RequestDivideDropItem(SrcInvenComp, SrcIdx, DividedCount);
	
	SetVisibility(ESlateVisibility::Collapsed);
	
	// CursorItem Clear
	CursorItem.Clear();
}

void UC_DivideItemWidget::HandleOnClickButtonExitBtn()
{
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(GetOwningPlayerPawn());
	
	if (!Player) return;
	
	UC_ItemSlotWidget* TargetSlot = Cast<UC_ItemSlotWidget>(TargetWidget);
	
	if (!TargetSlot) return;
	
	UC_InvenComponent* SrcInvenComp = CursorItem.SourceInvenComp;
	
	int32 SrcIdx = CursorItem.SourceSlotIndex;
	
	UC_InvenComponent* DstInvenComp = TargetSlot->GetAssociatedComponent();
	
	int32 DstIdx = TargetSlot->GetSlotIndex();
	
	Player->Server_RequestUnlockSlot(SrcInvenComp, SrcIdx);
	Player->Server_RequestUnlockSlot(DstInvenComp, DstIdx);
	
	SetVisibility(ESlateVisibility::Collapsed);
	
	// CursorItem Clear
	CursorItem.Clear();
	

}

void UC_DivideItemWidget::ReFocusDroppedCountText()
{
	if (DroppedCountText->HasKeyboardFocus())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UC_DivideItemWidget::ReFocusDroppedCountText);
		return;
	}
	
	DroppedCountText->SetKeyboardFocus();
}
