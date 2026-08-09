// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_MenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_MenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

private:
	// UMG의 Button_Socials와 이름이 일치해야 합니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Button_Socials;

	// UMG의 WBP_FriendList와 이름이 일치해야 합니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> WBP_FriendList;

	// 게임 종료 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> Button_Exit_Game;

	// 친구 버튼 클릭 시 호출될 함수
	UFUNCTION()
	void OnSocialsButtonClicked();

	// 게임 종료 버튼 함수
	UFUNCTION()
	void OnExitGameButtonClicked();

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

};