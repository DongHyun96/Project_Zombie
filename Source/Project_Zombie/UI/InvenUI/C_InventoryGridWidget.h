// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GlobalData.h"
#include "C_InventoryGridWidget.generated.h"

/**
 * 
 */
//struct FInventoryEntry; // FInventoryEntry 전방선언.

UCLASS()
class PROJECT_ZOMBIE_API UC_InventoryGridWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	// 인벤토리 전체를 강제로 새로고침하고 싶을 때 (인벤토리가 처음 열릴 때 등)
	void RefreshAllSlots(const TArray<FInventoryEntry>& InventoryItems);

	// 특정 슬롯만 콕 집어서 업데이트하고 싶을 때 (아이템 획득/소모/이동 시)
	UFUNCTION(BlueprintCallable)
	void RefreshSlotAt(int32 SlotIndex, const FInventoryEntry& ItemData);

	// InvenComponent의 델리게이트 신호를 받을 함수
	//UPROPERTY(ReplicatedUsing = OnRep_InventoryItems, EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	//void OnComponentSlotChanged(int32 SlotIndex, const FInventoryEntry& ItemData);
public:
	const TArray<class UC_ItemSlotWidget*>& GetSlotArr() const { return SlotWidgets; }
	
	void SetInvenComponent(class UC_InvenComponent* InventoryComponent);
	UC_InvenComponent* GetInvenComponent() const {return InvenComp;}
protected:
	// C_ItemSlot을 배치할 GridPanel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UUniformGridPanel* ItemGridPanel;

	// 에디터 디테일 패널에서 생성할 슬롯 위젯의 클래스를 지정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UC_ItemSlotWidget> SlotWidgetClass;

	// 총 몇칸을 만들 예정인지.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSlots = 0;

	// 몇 열짜리로 만들 것인지.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Column = 0;

	// 생성된 슬롯 위젯들을 순서대로 담아둘 동적 배열
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<class UC_ItemSlotWidget*> SlotWidgets;
	
	class UC_InvenComponent* InvenComp{};
};
