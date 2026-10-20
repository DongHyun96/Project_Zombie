// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_GameOverWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_GameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeOnInitialized() override;
	
	virtual void NativeConstruct() override;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:

	void ActivateWinningSequence();
	void ActivateLoseSequence();

	
	void UpdateExitToLobbyCountDownRemainSec(int32 _RemainSec);
	
private:
	
	void InitLocalPlayerRanking();
	
private:

	void HandleExitToLobbyTimer(float DeltaTime);

	UFUNCTION()
	void OnExitToLobbyButtonClicked();

protected:

	// 방장용 -> Exit Button 존재
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* WinningChickenAnimation{};

	// 방장용 -> Exit Button 존재
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* LoseAnimation{};	

protected: // 클라 전용 Exit Button 나오지 않는 Animations
	
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* Client_WinningChickenAnimation{};
	
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* Client_LoseAnimation{};

protected:
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerCharacterName{};

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RankingText{}; // 중단의 RankingText ( #(RankingNumber) )

	UPROPERTY(meta = (BindWidget))
	UTextBlock* KillCountText{};
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RankingTextTopRight{}; // Default "1"로 세팅되어 있음
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RankingTextTotalCharacterCount{};
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ExitToLobbyCountDownText{};

	//  ExitToLobbyButton 바인딩
	UPROPERTY(meta = (BindWidget))
	class UButton* ExitToLobbyButton{};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString m_LobbyLevelName = TEXT("GameLevel_LV_Loby_Safe_House");

private:

	// 최대 60초 기다린 이후 로비로 나가기
	float ExitToLobbyTimer = 60.f;
	int32 PrevExitToLobbyTimerForUI = 60;
	
	// 중복 로비 이동 실행 방지
	bool bIsExiting = false;

	uint8 m_Ranking{};
	
};
