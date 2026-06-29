// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GlobalData.h"
#include "C_ItemSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_ItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	//UFUNCTION(BlueprintCallable)
	void UpdateSlot(const FInventoryEntry& ItemData, const FItemData* CoreData);

	UFUNCTION(BlueprintCallable)
	void SetSlotIndex(int32 idx) { curSlotIdx = idx; }
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	class UImage* ItemSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 curSlotIdx = 0;
};
