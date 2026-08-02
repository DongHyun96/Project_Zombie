// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalEnum.h"
#include "Blueprint/UserWidget.h"
#include "C_MattersWidget.generated.h"

class UC_ItemUpgradeWidget;
struct FInventoryEntry;
class UC_MatterRowWidget;
//class UC_MatterRowWidget;
class UScrollBox;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_MattersWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateWidget(const FInventoryEntry& InEntry);
	
	void SetParentWidget(UC_ItemUpgradeWidget* ParentWidget) { ItemUpgradeWidget = ParentWidget; }
	
protected:
	// C_MatterRowWidget을 담아 사용 할 예정.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UScrollBox* MattersScrollBox = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UC_MatterRowWidget> MatterRowWidgetClass{};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UC_MatterRowWidget>> m_MatterRows{};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UC_ItemUpgradeWidget* ItemUpgradeWidget{}; 
};
