// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_NameTagWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_NameTagWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
public:

	UFUNCTION(BlueprintCallable)
	void ToggleNameTag(bool _Visible);

	UFUNCTION(BlueprintCallable)
	void SetNameTag(const FString& _PlayerName, const FColor& _PlayerColor);
	
protected:

	bool m_Visible{};
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NameText{};

	UPROPERTY(meta = (BindWidget))
	class UImage* DotImage{};
	
};
