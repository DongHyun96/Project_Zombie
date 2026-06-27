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
	void UpdateSlot(const FInventoryEntry& ItemData, const FItemData* CoreData);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget))
	class UImage* ItemSlot;
};
