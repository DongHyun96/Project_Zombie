// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_ItemUpgradeWidget.generated.h"

class UImage;
class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_ItemUpgradeWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemName = nullptr;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UImage* ItemIcon = nullptr;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UButton* ExitButton = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UButton* UpgradeBtn = nullptr;
	
};
