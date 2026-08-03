// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_PlayerUpMattersWidget.generated.h"

class AC_BasicPlayer;
class UC_PlayerStatUpgradeWidget;
class UC_MatterRowWidget;
class UScrollBox;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_PlayerUpMattersWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateWidget(AC_BasicPlayer* InUsePlayer);
public:
	void SetParentWidget(UC_PlayerStatUpgradeWidget* ParentWidget) { PlayerStatUpgradeWidget = ParentWidget; }
	
protected:
	// C_MatterRowWidget을 담아 사용 할 예정.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UScrollBox* PlayerUpMattersScrollBox = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UC_MatterRowWidget> MatterRowWidgetClass{};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UC_MatterRowWidget>> m_MatterRows{};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UC_PlayerStatUpgradeWidget* PlayerStatUpgradeWidget{};
};
