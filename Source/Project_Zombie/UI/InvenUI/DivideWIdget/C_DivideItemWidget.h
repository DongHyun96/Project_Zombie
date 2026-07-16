// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "Blueprint/UserWidget.h"
#include "C_DivideItemWidget.generated.h"


class USlider;
class UImage;
class UTextBlock;
class UEditableText;
struct FInventoryEntry;

UCLASS()
class PROJECT_ZOMBIE_API UC_DivideItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateWidget();
	
public:
	void SetCursorItem(const FCursorItem& InCursorItem) { CursorItem = InCursorItem; }
	
	// 몇개로 나눌지 입력이 들어왔을 때 최대 최소 사이에서 DividedCount가 정의 되도록함.
	UFUNCTION(BlueprintCallable)
	void CalculateDivideCount(int32 InCurCount);
	
	void UpdateSlider();
	
	void UpdateDroppedCount();
protected:
	virtual void NativeOnInitialized() override;
	// 
	//UFUNCTION()
	//void HandleOnClickedExitButton();
	
	// 슬라이드바를 움직이면 호출되는 함수
	UFUNCTION()
	void HandleOnValueChangedCountSlider(float InValue);

	// DroppedCountText가 수정되면 호출되는 함수.
	UFUNCTION()
	void HandleOnTextChangedDroppedCount(const FText& InText);
	
protected:
	// 딜레이 처리를 위한 타이머 핸들
	UFUNCTION()
	void ReFocusDroppedCountText();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UImage* ItemIcon{};
	
	// 아이템 이름 Text Block
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	UTextBlock* ItemNameTextBlock{};
	
	// 나눌 아이템의 갯수 Text Block
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	UEditableText* DroppedCountText{};
	
	// 드래그되어 나누기 시도를 당하고 있는 아이템의 갯수를 보여주는 Text Block
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	UEditableText* MaxItemCountText{};

	// 몇개로 나눌지 시각적으로 보고, 정할 수 있는 슬라이더 바
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	USlider* CountSlider{};
	
	// 현재 드래그된 아이템
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCursorItem CursorItem{};
	
	// 몇개로 나눌지 알려주는 값.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DividedCount = 0;
};
