// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "Blueprint/UserWidget.h"
#include "C_ItemStatsWidget.generated.h"

class UC_ItemStatRowWidget;
class UScrollBox;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_ItemStatsWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// FEquipmentCustomData 혹은 TArray<FCustomKeyVal>를 매개 변수로 받아와야 할 듯.
	void UpdateWidget(const FEquipmentCustomData* InCustomData);
	
	TArray<UC_ItemStatRowWidget*> GetItemStatRows() {return m_ItemStatRows;};
	
protected:
	// C_ItemStatRowWidget을 담아 사용 할 예정.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UScrollBox* ItemStatsScrollBox = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UC_ItemStatRowWidget> ItemStatRowWidgetClass{};
	
	TArray<TObjectPtr<UC_ItemStatRowWidget>> m_ItemStatRows{};
};
