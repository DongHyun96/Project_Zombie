// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "Blueprint/UserWidget.h"
#include "C_PlayerStatWidget.generated.h"

class UC_StatComponentBase;
enum class EFireMode : uint8;

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_PlayerStatWidget : public UUserWidget
{
	GENERATED_BODY()

	friend class UC_PlayerStatComponent;
	
public:
	
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void ToggleBoostBarColor(bool BoostExhausted);
	
	
	void BindCurHPUpdate(UC_StatComponentBase* InPlayerStatComponent);
public: // HPBar 및 BoostBar 관련
	
	/// <summary>
	/// Boost Bar 업데이트
	/// </summary>
	/// <param name="_Boost"> : 현재 Boost 량 </param>
	/// <param name="_MaxBoost"> : 최대 Boost 량 </param>
	/// <returns> : 잘못된 값이 들어왔을 경우 return false </returns>
	bool UpdateBoostBar(float _Boost, float _MaxBoost);
	
private:
	
	/// <summary>
	/// HP Bar Percent 업데이트 
	/// </summary>
	/// <param name="_HP"> : 현재 체력 </param>
	/// <param name="_MaxHP"> : 최대 체력 값 </param>
	/// <returns> : 잘못된 값이 들어왔을 경우 return false </returns>
	bool UpdateHPBar(float _HP, float _MaxHP);

	/// <summary>
	/// HP Bar Percent 업데이트
	/// </summary>
	/// <param name="_Ratio"> : HP 비율 </param>
	/// <returns> : 잘못된 값이 들어왔을 경우 return false </returns>
	void UpdateHPBarRatio(float _Ratio);
	
	void UpdateHPBarBoilerPlate(float _Ratio);
	
public:
	
	/// <summary>
	/// AmmoInfo 정보 Visibility 켜기/끄기 
	/// </summary>
	/// <param name="_Visible"> : 켜기/끄기 옵션 </param>
	/// <param name="_FireMode"> : 처음 표기할 FireMode / 켜기 옵션 시 default parameter 쓰지말고 해당 FireMode 넣어줄 것 </param>
	/// <param name="_MagazineAmmo"> : 처음 표기할 탄창에 남은 장탄수 / 켜기 옵션 시 default parameter 쓰지말고 해당 FireMode 넣어줄 것 </param>
	/// <param name="_LeftAmmoTotalCount"> : 처음 표기할 해당 총기 Type의 전체 보유한 탄약 수 / 켜기 옵션 시 default parameter 쓰지말고 해당 FireMode 넣어줄 것 </param>
	/// <returns> : 잘못된 인자값이 들어온 경우, 처리되지 않고 return false </returns>
	bool ToggleAmmoInfoVisibility
	(
		bool		_Visible,
		EFireMode	_FireMode 		= EFireMode::End,
		int32		_MagazineAmmo 	= 0,
		int32		_LeftAmmoTotalCount	= 0
	);

public:
	
	/// <summary>
	/// <para> 현재 탄창에 장착된 장탄수 업데이트 </para>
	/// <para> 주의 : ToggleAmmoInfoVisibility(_Visible true) 로 먼저 AmmoInfo 보여주고(무기를 꺼내는 등의 처리에서 호출이 되어야 함) </para>
	/// <para> -> 무기를 발사할 때, 장탄수 업데이트 시, 이 함수 이용할 것 </para>
	/// </summary>
	/// <param name="_AmmoCount"> : 탄창 장탄수 </param>
	void UpdateMagazineAmmoCount(int32 _AmmoCount);

	/// <summary>
	/// <para> 현재 탄창에 장착된 장탄수 업데이트 </para>
	/// <para> 주의 : ToggleAmmoInfoVisibility(_Visible true) 로 먼저 AmmoInfo 보여주고(무기를 꺼내는 등의 처리에서 호출이 되어야 함) </para>
	/// <para> -> 무기를 발사할 때, 장탄수 업데이트 시, 이 함수 이용할 것 </para>
	/// </summary>
	/// <param name="_LeftAmmoTotalCount"> : 해당 무기의 사용할 수 있는 총 탄약 수 </param>
	void UpdateLeftAmmoTotalCount(int32 _LeftAmmoTotalCount);

public:
	
	/// <summary>
	/// FireMode 변경 
	/// </summary>
	/// <returns> : 현재 AmmoInfo Visibility false이거나 새로 들어온 FireMode가 이미 보여지는 FireMode인 경우, Update 하지 않음(return false) </returns>
	bool UpdateFireMode(EFireMode _NewFireMode);
	
private:
	
	/// <summary>
	/// 특정 ProgressBar 이번 Tick Lerp 처리 
	/// </summary>
	void LerpProgressBar(class UProgressBar* _TargetProgressBar, float _LerpAlphaSpeed, float _DestRatio, float _DeltaTime);

	/// <summary>
	/// 현재 보여주고 있는 MagazineText 내용 업데이트
	/// </summary>
	void SetCurrentShowingMagazineText(int32 _AmmoAmount);

	
	/// <summary>
	/// 현재 보여주고 있는 LeftAmmoText 내용 업데이트 
	/// </summary>
	void SetCurrentShowingLeftAmmoText(int32 _AmmoAmount);
	
	/// <summary>
	/// 현재 보여주고 있지 않는 Hidden mag text 내용, 보여지는 Mag Text 내용과 동일하게 세팅 
	/// </summary>
	void PasteCurrentShowingMagTextToHidden();

	/// <summary>
	/// 현재 보여주고 있지 않는 Hidden LeftAmmoText 내용, 보여지는 LeftAmmoText 내용과 동일하게 세팅
	/// </summary>
	void PasteCurrentShowingLeftAmmoTextToHidden();
	
	/* Main HPBar 관련 */
protected:
	
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* MainHPBar{};
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* DamageIndicatorBar{};
	
private:

	// DamageBar Lerp 처리를 해야하는지 여부
	bool m_bEnableDamageBarLerp{};
	
	// MainHPBar Percent Lerp 목적지 (이 값이 항상 실질적인 ProgressBar의 현재 값으로 세팅되어야 한다)
	// 첫 시작 체력은 모두 채워진 상태로, 기본값 1을 설정해주어야 한다.
	float m_MainHPBarPercentLerpDest = 1.f;
	
	float m_DamageIndicatorPercentLerpDest{};

	/* Main HPBar 관련 */
protected:
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* BoostBar{};
	
private:
	
	float m_BoostBarPercentLerpDest{};
	
	/* Ammo Info 관련 */

private: // 전체 AmmoInfo Animation Visibility Toggle animation 마지막으로 재생한 방향 저장 

	// 처음 상태 감춰진 상태로, Reverse방향이 감추는 Animation 재생 방향 -> 이 값 true로 시작처리함
	bool m_bAmmoInfoPlayedReverseFlag = true;
	
private: // 현재 Main으로 표기중인 정보

	// 현재 표기중인 FireMode Type
	EFireMode m_CurrentShowingFireMode{};

	// True, false로 처리할 예정
	bool m_bCurrentShowingMagTextIdx{}; 
	bool m_bCurrentShowingLeftAmmoTextIdx{};
	
	TArray<class UTextBlock*> m_MagazineTexts{};
	TArray<UTextBlock*> m_LeftAmmoTexts{};

	
	
protected: // Change FireMode Animations

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* SingleToBurst{};
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* SingleToAuto{};
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* BurstToSingle{};
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* BurstToAuto{};
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* AutoToSingle{};
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* AutoToBurst{};
	
protected: // ShowAmmoInfos Animations (역으로 재생 시, 감추는 Animation으로 처리 가능)

	// 주의 : MagText LeftAmmoText 둘 다 1번으로 사용
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ShowAmmoInfos_SingleMode{};
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ShowAmmoInfos_BurstMode{};
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ShowAmmoInfos_AutoMode{};

protected: // UpdateAmmoAnimations

	// Ammo2 에서 Ammo1 보여주기 처리로 되는 Animation
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* UpdateLeftAmmo1{};

	// Ammo2에서 Ammo1 보여주기 처리로 되는 Animation
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* UpdateLeftAmmo2{};

	// Mag2에서 Mag1 보여주기 처리로 되는 Animation
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* UpdateMagazineText1{};
	
	// Mag1에서 Mag2 보여주기 처리로 되는 Animation
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* UpdateMagazineText2{};
	
private:
	
	TMap<EFireMode, UWidgetAnimation*> m_ShowAmmoInfosAnims{};

	TArray<UWidgetAnimation*> m_UpdateMagazineTextAnimations{};
	TArray<UWidgetAnimation*> m_UpdateTotalLeftAmmoAnimations{};
	
protected: // 실질적인 TextBlock들

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MagazineText1{};
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MagazineText2{};
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* LeftAmmoText1{};
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* LeftAmmoText2{};
	
};
