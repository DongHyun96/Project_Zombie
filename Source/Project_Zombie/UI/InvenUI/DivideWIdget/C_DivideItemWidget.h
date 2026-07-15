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
	
	UFUNCTION(BlueprintCallable, Category = "CustomDelay", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	void CustomDelay(UObject* WorldContextObject, float Duration, FLatentActionInfo LatentInfo);
	
	UFUNCTION(BlueprintImplementableEvent)
	void FocusAndSelectAllText();
	
	
protected:
	// 딜레이 처리를 위한 타이머 핸들
	//FTimerHandle FocusTimerHandle;
	UFUNCTION()
	void ReFocusDroppedCountText();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UImage* ItemIcon{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	UTextBlock* ItemNameTextBlock{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	UEditableText* DroppedCountText{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	UEditableText* MaxItemCountText{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	USlider* CountSlider{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCursorItem CursorItem{};
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DividedCount = 0;
};
