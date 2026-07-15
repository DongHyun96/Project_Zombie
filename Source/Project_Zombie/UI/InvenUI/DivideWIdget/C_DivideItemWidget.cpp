// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InvenUI/DivideWIdget/C_DivideItemWidget.h"

#include "DelayAction.h"
#include "Components/EditableText.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "Kismet/KismetSystemLibrary.h"
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
	
	CountSlider->SetMinValue(1.0f);
	CountSlider->SetMaxValue(MaxCount);
	MaxItemCountText->SetText(FText::FromString(FString::FromInt(MaxCount)));
	UpdateDroppedCount();
	UpdateSlider();
	
	
	DroppedCountText->SetSelectAllTextWhenFocused(true);
	
	DroppedCountText->SetFocus();
	SetVisibility(ESlateVisibility::Visible);
}

void UC_DivideItemWidget::CalculateDivideCount(int32 InCurCount)
{
	UC_Util::Print(InCurCount,FColor::Blue, 10.f);
	
	if (InCurCount <= 0)
	{
		DividedCount = 1;
		//FocusAndSelectAllText();
		UC_Util::Print(DividedCount,FColor::Black, 10.f);
		UC_Util::Print("Calculate",FColor::Black, 10.f);
		
		CountSlider->SetFocus();
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
	//if (CountSlider->GetValue() == DividedCount) return;
	UC_Util::Print(DividedCount,FColor::Red, 10.f);
	CountSlider->SetValue(static_cast<float>(DividedCount));
	//CountSlider->SetFocus();
}

void UC_DivideItemWidget::UpdateDroppedCount()
{
	//if (DroppedCountText->GetText().ToString() == FString::FromInt(DividedCount)) return; 
	UC_Util::Print(DividedCount,FColor::Emerald, 10.f);
	DroppedCountText->SetText(FText::FromString(FString::FromInt(DividedCount)));
	//DroppedCountText->SetSelectAllTextWhenFocused(false);
	//DroppedCountText->SetFocus();
}

void UC_DivideItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UC_Util::Print(DividedCount,FColor::Magenta, 10.f);
	CountSlider->OnValueChanged.AddUniqueDynamic(
	this,
	&UC_DivideItemWidget::HandleOnValueChangedCountSlider);

	DroppedCountText->OnTextChanged.AddUniqueDynamic(
		this,
		&UC_DivideItemWidget::HandleOnTextChangedDroppedCount);
}

void UC_DivideItemWidget::HandleOnValueChangedCountSlider(float InValue)
{
	UC_Util::Print(DividedCount,FColor::Red, 10.f);
	DividedCount = FMath::TruncToInt32(InValue);
	//if (CountSlider->GetValue() == DividedCount) return;
	UpdateSlider();
	UpdateDroppedCount();
}

void UC_DivideItemWidget::HandleOnTextChangedDroppedCount(const FText& InText)
{
	FString TextString = InText.ToString();
    
	// [수정됨] 텍스트가 완전히 비어있다면 0을 넘겨서 'CalculateDivideCount'가 0 이하 예외 처리를 하도록 유도합니다.
	//int32 TargetValue = TextString.IsEmpty() ? 0 : FCString::Atoi(*TextString);
	UC_Util::Print(TextString,FColor::Red, 10.f);
	if (TextString == FString(""))
	{
		UC_Util::Print("Text Is Null!",FColor::Red, 10.f);
	}
	//if (TextString == FString(""))
	//{
	//	DividedCount = 1;
	//	
	//	UpdateDroppedCount();
	//	
	//	DroppedCountText->SetFocus();
	//	
	//	return;
	//}
	
	CalculateDivideCount(FCString::Atoi(*TextString));
	
	//if (DroppedCountText->GetText().ToString() == FString::FromInt(DividedCount)) return; 
	
	UpdateSlider();
	UpdateDroppedCount();
}

void UC_DivideItemWidget::CustomDelay(UObject* WorldContextObject, float Duration, FLatentActionInfo LatentInfo)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
        
		// 언리얼 엔진 내부의 Delay 액션을 직접 등록해 실행합니다.
		LatentActionManager.AddNewAction(
			LatentInfo.CallbackTarget, 
			LatentInfo.UUID, 
			new FDelayAction(Duration, LatentInfo)
		);
	}
}

void UC_DivideItemWidget::ReFocusDroppedCountText()
{
	DroppedCountText->SetFocus();
	UC_Util::Print("ReFocus",FColor::Red, 10.f);
}

//void UC_DivideItemWidget::HandleOnClickedExitButton()
//{
//}
