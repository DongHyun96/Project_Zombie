// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_GameMainHUD.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_GameMainHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:

	UFUNCTION(BlueprintCallable)
	bool UpdateHPBar(float _HP, float _MaxHP);

	UFUNCTION(BlueprintCallable)
	bool UpdateBoostBar(float _Boost, float _MaxBoost);
	
protected:

	UPROPERTY(meta = (BindWidget))
	class UC_PlayerStatWidget* PlayerStatWidget{};

};
