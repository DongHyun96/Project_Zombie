// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "Blueprint/UserWidget.h"
#include "C_BaseItemSlotWidget.generated.h"

class UC_DragDropOperation;
class UC_InvenComponent;
class UImage;
class UTextBlock;

UCLASS()
class PROJECT_ZOMBIE_API UC_BaseItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void UpdateSlot(const FInventoryEntry& ItemData, const FItemData* CoreData);

	UFUNCTION(BlueprintCallable)
	void SetSlotIndex(int32 idx) { curSlotIdx = idx; }
	int32 GetSlotIndex() const { return curSlotIdx; }

	void SetAssociatedComponent(UC_InvenComponent* InComp) { AssociatedInvenComp = InComp; }
	UC_InvenComponent* GetAssociatedComponent() const { return AssociatedInvenComp; }

	// 마우스 및 드래그 공통 이벤트
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	void ItemIconSetOpacity(float InOpacity);
	void ItemIconSetVisibility(ESlateVisibility InVisibility);

protected:
	void InitDragVisual(UC_DragDropOperation* InDragDropOp);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UImage* ItemIcon{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UImage* BackGroundImage{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemCountText{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 curSlotIdx = 0;

	UPROPERTY()
	UC_InvenComponent* AssociatedInvenComp = nullptr;
};