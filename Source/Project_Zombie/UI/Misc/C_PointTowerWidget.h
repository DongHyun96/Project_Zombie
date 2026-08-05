// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_PointTowerWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_PointTowerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	void SetPercentText(uint8 _Percent);
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ConqueredPercentText{};
	
};
