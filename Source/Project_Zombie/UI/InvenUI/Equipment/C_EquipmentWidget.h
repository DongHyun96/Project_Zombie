// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "Blueprint/UserWidget.h"
#include "C_EquipmentWidget.generated.h"

class UC_EquipmentItemSlotWidget;
class UC_InvenComponent;

UCLASS()
class PROJECT_ZOMBIE_API UC_EquipmentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 장비창 초기화 함수 (InvenComponent 및 인덱스 매핑)
	void InitEquipmentWidget(UC_InvenComponent* InInvenComp);

	// 장비 인벤토리 영역 데이터가 갱신될 때 호출
	void RefreshEquipmentAllSlots();
	
	UFUNCTION(BlueprintCallable)
	void RefreshEquipmentSlotAt(int32 InIndex, const FInventoryEntry& ItemData);

protected:
	virtual void NativeConstruct() override;

protected:
	// 이 위젯이 바인딩될 플레이어의 InvenComponent
	UPROPERTY()
	UC_InvenComponent* AssociatedInvenComp = nullptr;

	// Designer에서 배치할 장비 슬롯들 (BindWidget 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	UC_EquipmentItemSlotWidget* Slot_MainGun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	UC_EquipmentItemSlotWidget* Slot_Melee;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	UC_EquipmentItemSlotWidget* Slot_Throwable;

	//UPROPERTY(meta = (BindWidget))
	//UC_ItemSlotWidget* Slot_SubWeapon;

	// 빠른 관리를 위한 슬롯 배열
	UPROPERTY()
	TArray<UC_EquipmentItemSlotWidget*> EquipmentSlots;
};