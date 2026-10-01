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

	/// <summary>
	/// 아이템 파밍 시 띄울 로그 추가
	/// </summary>
	/// <param name="_ItemRowName"> : 해당 ItemRowName </param>
	/// <param name="_ItemPickUpCount"> : 파밍한 갯수 </param>
	/// <returns> : Valid하지 않은 값이 들어왔다면 return false </returns>
	bool AddPlayerWarningLog(const FName& _ItemRowName, int32 _ItemPickUpCount);

	void ToggleGameStartPanel(bool _Visible);
	void UpdateGameStartLeftTime(int32 _Time);
	void ShowMainInstruction(const FString& _Construction);

public:
	
	/// <summary>
	/// 좌상단 CurSequence 정보 모두 가리기 처리 
	/// </summary>
	void HideAllCurSequenceInfo();
	
	/// <summary>
	/// Time Remain Info(좌상단) Visibility toggle 
	/// </summary>
	void ToggleTimeRemainInfo(bool _Visible, int32 _RemainTime = 0);

	/// <summary>
	/// 남은 시간 Text 내용 수정
	/// </summary>
	/// <param name="_RemainTime"> : 남은 시간(초) int32 </param>
	void SetTimeRemainInfo(int32 _RemainTime);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="_CurSeqTowerTotalCount"> : 현재 Sequence의 활성화될 PointTower 개수 </param>
	/// <returns> : Invalid한 개수가 들어오면 return false(현재 위젯 설정 상 4개까지 가능) </returns>
	bool ShowTowerConqueredInfo(uint8 _CurSeqTowerTotalCount);
	
	/// <summary>
	/// 해당 idx의 PointTower 점령 퍼센트 정보 수정 
	/// </summary>
	/// <returns> : Valid한 Idx가 아닌 경우, return false </returns>
	bool SetTowerConqueredInfo(int _Idx, uint8 _Percent);

public: /* PointTower 방면 Indicator 관련 함수 */

	/// <summary>
	/// 해당 PointTower의 위치 정보 방면을 표기할 Indicator 등록 
	/// </summary>
	/// <returns> : 이미 동일한 PointTower를 가리키는 Active한 Indicator가 존재한다면 return false, 또는 등록 가능한 Indicator 개수가 부족하다면 return false </returns>
	bool RegisterPointTowerIndicator(class AC_PointTower* _PointTower);

	/// <summary>
	/// 해당 PointTower의 Indicator 등록 해제 
	/// </summary>
	/// <returns> : 등록된 PointTower가 없다면 return false </returns>
	bool DeRegisterPointTowerIndicator(AC_PointTower* _PointTower);
	
private:
	
	/// <summary>
	/// 해당 idx의 PointTower 정보 Visibility toggle
	/// </summary>
	/// <returns> : Valid한 Idx(Sequence당 최대 PointTower는 4개로 가정을 함)가 아닌 경우, return false </returns>
	bool ToggleTowerConqueredInfo(uint8 _Idx, bool _Visible);
	
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
	
protected: /* Time remaining 정보 관련(현재 Sequence의 점령 남은 시간 정보 UI) */

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* RemainTimeHZBox{};
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeRemainMin{};

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeRemainColon{};
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeRemainSec{};

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShowRemainTime{};

	FSlateColor m_RemainTimeOriginColor{};
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FSlateColor m_TimeUrgentColor{};
	
protected: /* Tower conquered 퍼센트 정보 UI 관련 */

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* TowerConqueredHZBox0{};
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* TowerConqueredHZBox1{};
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* TowerConqueredHZBox2{};
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* TowerConqueredHZBox3{};
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TowerConqueredPercent0{};
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TowerConqueredPercent1{}; 
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TowerConqueredPercent2{}; 
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TowerConqueredPercent3{}; 

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShowTowerConquered0{};
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShowTowerConquered1{};
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShowTowerConquered2{};
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShowTowerConquered3{};

private:
	
	UPROPERTY()
	TArray<UCanvasPanel*> m_TowerConqueredHZBoxes{};
	
	UPROPERTY()
	TArray<UTextBlock*> m_TowerConqueredPercentTexts{};
	
	UPROPERTY()
	TArray<UWidgetAnimation*> m_ShowTowerConqueredInfoAnims{};

protected:

	UPROPERTY(meta = (BindWidget))
	class UC_PointIndicatorManagerWidget* PointIndicatorManagerWidget{};
	
};
