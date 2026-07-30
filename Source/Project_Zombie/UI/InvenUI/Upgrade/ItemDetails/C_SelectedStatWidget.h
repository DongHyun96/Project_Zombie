// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "Blueprint/UserWidget.h"
#include "C_SelectedStatWidget.generated.h"

class UTextBlock;
/**
 *
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_SelectedStatWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void UpdateWidget(EUpgradableStats InStatType, const float& InCurStat, const float& InMaxStat);
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* StatName = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* CurStat = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* NextStat = nullptr;
};
