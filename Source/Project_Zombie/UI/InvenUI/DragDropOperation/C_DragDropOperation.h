// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "Blueprint/DragDropOperation.h"
#include "UI/InvenUI/C_ItemSlotWidget.h"
#include "C_DragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_DragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
public:
	void SetItmeEntry(FInventoryEntry InItemEntry){ItemEntry = InItemEntry;}
	
	void SetSlotIndex(int InSlotIndex){SlotIndex = InSlotIndex;}
	
	int32 GetSlotIndex(){return SlotIndex;}
protected:
	FInventoryEntry ItemEntry;
	int32 SlotIndex;
};
