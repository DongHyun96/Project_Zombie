// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/DivideWIdget/C_DivideItemWidget.h"

#include "Components/EditableText.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "GameModeAndManager/C_ItemManager.h"
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
	
	DroppedCountText->SetSelectAllTextWhenFocused(true);
	
	DroppedCountText->SetKeyboardFocus();
	
	SetVisibility(ESlateVisibility::Visible);
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

void UC_DivideItemWidget::ReFocusDroppedCountText()
{
	if (DroppedCountText->HasKeyboardFocus())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UC_DivideItemWidget::ReFocusDroppedCountText);
		return;
	}
	
	DroppedCountText->SetKeyboardFocus();
}