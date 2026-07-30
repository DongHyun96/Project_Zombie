// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_InventoryWidget.generated.h"

class UC_ItemUpgradeWidget;
class UC_EquipmentWidget;
class UC_DivideItemWidget;
class UC_DragDropOperation;

UCLASS()
class PROJECT_ZOMBIE_API UC_InventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
public:
	void ShowDivideEntryWidget();
	
	void ShowDivideItemWidget();
	
	void ShowUpgradeWidget();
	
	void CloseUpgradeWidget();

public:
	class UC_InventoryGridWidget* GetPlayerGridWidget() { return PlayerGridWidget; }
	
	UC_InventoryGridWidget* GetStorageGridWidget() { return StorageGridWidget; }
	
	UC_EquipmentWidget* GetEquipmentWidget() { return EquipmentWidget; }
	
	UC_DivideItemWidget* GetDivideItemWidget() { return DivideItemWidget; }
	
	UC_ItemUpgradeWidget* GetItemUpgradeWidget() {return UpgradeWidget;}
	
	virtual void SetVisibility(ESlateVisibility InVisibility) override;
	
protected:
	// Player의 아이템 슬롯을 가지고 있을 그리드 위젯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UC_InventoryGridWidget* PlayerGridWidget = nullptr;
	
	// Storage(창고)의 아이템 슬롯을 가지고 있을 그리드 위젯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UC_InventoryGridWidget* StorageGridWidget = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UC_ItemUpgradeWidget* UpgradeWidget = nullptr;
	
	// Equipment(장비창)의 아이템 슬롯을 가지고 있을 그리드 위젯.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UC_EquipmentWidget* EquipmentWidget = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UC_DivideItemWidget* DivideItemWidget = nullptr;

};
