// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_BaseItemSlotWidget.h"
#include "C_GridItemSlotWidget.generated.h"

class UC_DragDropOperation;
class UC_InventoryGridWidget;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_GridItemSlotWidget : public UC_BaseItemSlotWidget
{
	GENERATED_BODY()
public:
	void SetGridWidget(UC_InventoryGridWidget* GridWidget) { ParentGrid = GridWidget; }
	UC_InventoryGridWidget* GetGridWidget() const { return ParentGrid; }

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

protected:
	UPROPERTY()
	UC_InventoryGridWidget* ParentGrid = nullptr;
};
