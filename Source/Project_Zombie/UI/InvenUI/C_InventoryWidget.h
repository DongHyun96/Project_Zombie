// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_InventoryWidget.generated.h"


UCLASS()
class PROJECT_ZOMBIE_API UC_InventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
public:
	class UC_InventoryGridWidget* GetGridWidget() { return GridWidget; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UC_InventoryGridWidget* GridWidget = nullptr;
};
