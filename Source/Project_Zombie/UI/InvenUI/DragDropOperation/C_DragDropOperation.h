// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "Blueprint/DragDropOperation.h"
#include "UI/InvenUI/C_GridItemSlotWidget.h"
#include "C_DragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_DragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
public:
	void SetItemEntry(FInventoryEntry InItemEntry){ItemEntry = InItemEntry;}
	
	void SetSlotIndex(int InSlotIndex){SlotIndex = InSlotIndex;}
	
	int32 GetSlotIndex() {return SlotIndex;}
	
	// 출발지 컴포넌트 세팅 함수
	void SetSourceComponent(class UC_InvenComponent* InComp) { SourceInvenComp = InComp; }
	class UC_InvenComponent* GetSourceComponent() const { return SourceInvenComp; }
protected:
	FInventoryEntry ItemEntry;
	int32 SlotIndex;

	// 드래그가 시작된 인벤토리 컴포넌트
	UPROPERTY()
	class UC_InvenComponent* SourceInvenComp = nullptr;
};
