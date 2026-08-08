// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_MiniHPBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_MiniHPBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	
	/// <summary>
	/// 해당 Player의 정보를 토대로 이 MiniHPBarWidget 활성화 처리 -> 주의 : 아직 PlayerProfile 정보가 초기화되지 않은상황에서 호출해버리면, 없는 정보로 세팅됨 
	/// </summary>
	void Activate(class AC_BasicPlayer* _TargetPlayer);
	
	void UpdateHPBar(float _Ratio);
	
private:
	
	void LerpProgressBar(class UProgressBar* _TargetProgressBar, float _LerpAlphaSpeed, float _DestRatio, float _DeltaTime);
	
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	class UImage* PlayerColorImage{};
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerNameText{};
	
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
	
};
