// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_MatterRowWidget.generated.h"

class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_MatterRowWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void UpdateWidget(UTexture2D* InItemIcon, const FText& InItemName, const int32& InItemCount);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UImage* ItemIcon = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemName = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemCount = nullptr;
};
