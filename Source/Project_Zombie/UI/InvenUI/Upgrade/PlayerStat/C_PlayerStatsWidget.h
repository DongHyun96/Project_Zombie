// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_PlayerStatsWidget.generated.h"

class UC_PlayerStatUpgradeWidget;
class UC_PlayerStatRowWidget;
class UScrollBox;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_PlayerStatsWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateWidget(const FInventoryEntry& InEntry);
	
	TArray<UC_PlayerStatRowWidget*> GetItemStatRows() {return m_PlayerStatRows;};
	
	void SetParentWidget(UC_PlayerStatUpgradeWidget* ParentWidget) { PlayerStatUpgradeWidget = ParentWidget; }
protected:
	// C_ItemStatRowWidget을 담아 사용 할 예정.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UScrollBox* PlayerStatsScrollBox = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UC_PlayerStatRowWidget> PlayerStatRowWidgetClass{};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UC_PlayerStatRowWidget>> m_PlayerStatRows{};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UC_PlayerStatUpgradeWidget* PlayerStatUpgradeWidget{};
};
