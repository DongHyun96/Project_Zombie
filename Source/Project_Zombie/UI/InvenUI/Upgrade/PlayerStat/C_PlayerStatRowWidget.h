// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_PlayerStatRowWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_PlayerStatRowWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:

	void UpdateWidget(const FText& InStatName, const float& InStatValue, const int32& InCurGrade, const int32& InMaxGrade);
	
	const FText& GetTargetStat() {return TargetStatName;}
	
	UTextBlock* GetSelectedRow() { return SelectedRow; }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* SelectedRow = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* StatName = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* StatValue = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* CurGrade = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* MaxGrade = nullptr;

	FText TargetStatName{};
};
