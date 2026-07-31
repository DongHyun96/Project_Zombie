// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalEnum.h"
#include "UI/InvenUI/C_BaseItemSlotWidget.h"
#include "C_EquipmentItemSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_EquipmentItemSlotWidget : public UC_BaseItemSlotWidget
{
	GENERATED_BODY()
public:
	void SetAllowedItemType(EItemType InType) { AllowedItemType = InType; }
	EItemType GetAllowSlotType() const { return AllowedItemType; }
	
protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

protected:
	// 장비 슬롯처럼 특정 아이템 타입만 들어와야 하는 경우 지정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EItemType AllowedItemType = EItemType::MAX;
};
