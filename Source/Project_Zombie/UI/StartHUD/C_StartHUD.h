// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_StartHUD.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_StartHUD : public UUserWidget
{
	GENERATED_BODY()

protected:
	/*UPROPERTY(meta=(BindWidget))
	class UButton* StartBtn;

	UPROPERTY(meta = (BindWidget))
	class UButton* QuitBtn;*/

	TMap<FString, class UWidgetAnimation*>	m_mapAnim;

	/*UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* OnHoverScaleUp;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* UnhoverScaleDown;*/


public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION()
	void StartButtonClicked();

	UFUNCTION()
	void QuitButtonClicked();

	UFUNCTION()
	void StartButtonHovered();

	UFUNCTION()
	void StartButtonUnhovered();



};
