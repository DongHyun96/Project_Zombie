// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "Blueprint/UserWidget.h"
#include "C_ItemStatRowWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_ItemStatRowWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	// TODO : 나중에 해당 StatValue까지 표현하게 만들기.
	//void UpdateWidget(EUpgradableStats InStatType, const float& InStatValue, const int32& InCurGrade, const int32& InMaxGrade);
	void UpdateWidget(EUpgradableStats InStatType, const int32& InCurGrade, const int32& InMaxGrade);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* StatName = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* StatValue = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* CurGrade = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* MaxGrade = nullptr;

	EUpgradableStats TargetStat = EUpgradableStats::None;
};
