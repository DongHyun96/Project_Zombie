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

class UC_InvenComponent;
class UC_GridItemSlotWidget;
class UC_InventoryWidget;

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
	
	// 최초 1회 초기화, 아이템 슬롯 생성 및 배열에 넣어줌.
	virtual void NativeOnInitialized() override;
	//virtual bool Initialize() override;
public:
	const TArray<UC_GridItemSlotWidget*>& GetSlotArr() const { return SlotWidgets; }

	// 창고, 플레이어에 InvenComponent를 추가할 때 델리게이트를 연결하기 위한 함수.
	void SetInvenComponent(UC_InvenComponent* InventoryComponent);
	
	UC_InvenComponent* GetInvenComponent() const {return InvenComp;}
	
	void SetParentWidget(UC_InventoryWidget* InInvenWidget) {ParentWidget = InInvenWidget;}
	
	UC_InventoryWidget* GetParentInventoryWidget() const {return ParentWidget;}
	
protected:
	void SetSlotStartIdx(int32 InSlotStartIdx);	
protected:
	// C_ItemSlot을 배치할 GridPanel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UUniformGridPanel* ItemGridPanel;

	// 에디터 디테일 패널에서 생성할 슬롯 위젯의 클래스를 지정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UC_GridItemSlotWidget> SlotWidgetClass;

	// 총 몇칸을 만들 예정인지.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSlots = 0;

	// 몇 열짜리로 만들 것인지.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Column = 0;

	// 생성된 슬롯 위젯들을 순서대로 담아둘 동적 배열
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UC_GridItemSlotWidget*> SlotWidgets;
	
	UPROPERTY()
	UC_InvenComponent* InvenComp{};
	
	UPROPERTY()
	UC_InventoryWidget* ParentWidget{};
	
	// 현재 적용된 시작 슬롯 인덱스 (기본값 0)
	int32 SlotStartIdx = 0;
};
