// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_MattersWidget.generated.h"

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
	
	void UpdateWidget(TArray<const FName> InItemRowNames);
protected:
	// C_MatterRowWidget을 담아 사용 할 예정.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UScrollBox* MattersScrollBox = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UC_MatterRowWidget> MatterRowWidgetClass;
	
	TArray<TObjectPtr<UC_MatterRowWidget>> m_MatterRows{};
};
