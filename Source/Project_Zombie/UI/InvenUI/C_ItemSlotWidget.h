// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GlobalData.h"
#include "C_ItemSlotWidget.generated.h"

class UC_DragDropOperation;
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
	
	void SetGridWidget(class UC_InventoryGridWidget* GridWidget) { ParentGrid = GridWidget; }
	
	// 이 슬롯이 소속된 인벤토리 컴포넌트 세팅 및 가져오기
	void SetAssociatedComponent(class UC_InvenComponent* InComp) { AssociatedInvenComp = InComp; }
	
	// 위젯을 클릭하는 마우스 버튼 감지 함수
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 위젯에 대한 드래그 감지 함수
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	
	// 위젯에게 드래그된 것이 드롭되었을 때 함수
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
protected:
	// 드래그시 보이는 이미지 생성 및 이미지 위치설정
	void InitDragVisual(UC_DragDropOperation* InDragDropOp);
	
protected:
	// 아이템 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	class UImage* ItemSlot;

	// ItemSlot 자신의 Inventory에서 어떤 Index인지 기억.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 curSlotIdx = 0;
	
	UC_InventoryGridWidget* ParentGrid = nullptr;
	
	// 이 슬롯이 참조하는 실제 데이터 컴포넌트
	UPROPERTY()
	class UC_InvenComponent* AssociatedInvenComp = nullptr;
};
