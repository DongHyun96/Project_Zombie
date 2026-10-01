// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GlobalEnum.h"
#include "C_SkinWidget.generated.h"

class UButton;
class AC_BasicPlayer;

UCLASS()
class PROJECT_ZOMBIE_API UC_SkinWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetUsePlayer(AC_BasicPlayer* InPlayer);

protected:
	virtual void NativeConstruct() override;

private:
	void SelectSkin(EPlayerSkin InSkin);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Origin;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Purple;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Red;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Green;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Blue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Close;

	UPROPERTY()
	TObjectPtr<AC_BasicPlayer> m_UsePlayer;

private:
	UFUNCTION()
	void OnClickOrigin();

	UFUNCTION()
	void OnClickPurple();

	UFUNCTION()
	void OnClickRed();

	UFUNCTION()
	void OnClickGreen();

	UFUNCTION()
	void OnClickBlue();

	UFUNCTION()
	void OnClickClose();
};
