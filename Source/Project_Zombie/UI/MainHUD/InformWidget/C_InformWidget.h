// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_InformWidget.generated.h"

enum class EQueueLogType : uint8
{
	PlayerWarningLog,
	TopKillFeedLog
};

/**
 * 인게임 로그, 킬로그, 주요 정보 알림창 역할 Widget 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_InformWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;


public:
	
	/// <summary>
	/// Player Warning Log 추가
	/// </summary>
	/// <param name="WarningLog"> : Warning log </param>
	/// <param name="_TextColor"> : TargetLog 색상 </param>
	/// <returns> : 제대로 추가되지 않았다면 return false </returns>
	bool AddPlayerWarningLog(const FString& WarningLog, const FColor& _TextColor);

	bool AddEquippedWeaponLog(const FName& _WeaponItemRowName);
	
	void ToggleGameStartPanel(bool _Visible);
	void UpdateGameStartLeftTime(int32 _Time);
	void ShowMainInstruction(const FString& _Construction);
	
private:
	
	/// <summary>
	/// Fade Out Start 처리된 Widget들 FadeOut 처리 
	/// </summary>
	void HandleLogFadeOut(const float& DeltaTime);

	/// <summary>
	/// Log Queue 방식으로 처리 Handling
	/// </summary>
	void HandleLogQueuePositionsAndDefaultAlpha(const float& DeltaTime);

	/// <summary>
	/// 로그에 LifeTimer 등록
	/// </summary>
	/// <param name="Log"> : TargetLog </param>
	/// <param name="TotalLifeTime"> : 총 수명 시간 </param>
	void ApplyNewLifeTimerToLog(UWidget* Log, float TotalLifeTime);
	
	/// <summary>
	/// FadeOut 효과 처리 시작하기 
	/// </summary>
	void StartFadeOut(UWidget* TargetWidget) { FadeOutLogs.Add(TargetWidget); }

	
private:
	
	UFUNCTION()
	void OnLogLifeTimeExpired(UWidget* TargetWidget);

private:

	UFUNCTION()
	void OnInvenSlotChanged(int32 _SlotIndex, const struct FInventoryEntry& _ItemData);
	
private: // Player Warning Log 관련

	TArray<class UTextBlock*>			PlayerWarningLogTexts{};
	TArray<class UCanvasPanelSlot*>		PlayerWarningLogTextPanels{};
	TArray<FVector2D>					PlayerWarningLogEachPositions{}; // Player Warning Log 각 위치의 초기 Position 값
	TArray<int>							PlayerWarningLogSequence{}; // 현재 Log Panel들의 순서 (차례로 밑에서부터 위로)

private:
	
	TMap<UWidget*, FTimerHandle> LogLifeTimers{};	// Log Spawn된 이 후, FadeOut처리되기 이전까지의 수명처리 담당
	TSet<UWidget*>				 FadeOutLogs{};		// FadeOut 처리시킬 Widget들
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameStartTimerText{};

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MainInstructionText{};

	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* GameStartsTimerPanel{};  
	
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShowMainInstructionAnim{};

	// Key : ItemRowName | Value : 실질적인 아이템 명(인게임 플레이 아이템 이름)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TMap<FName, FString> m_ItemNameMap{}; 
	
private:
	
	FTimerHandle m_TimerInvenGameLogRegister{};
	
};
