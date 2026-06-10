// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_PlayerStatWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_PlayerStatWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	
	/// <summary>
	/// HP Bar Percent 업데이트 
	/// </summary>
	/// <param name="_HP"> : 현재 체력 </param>
	/// <param name="_MaxHP"> : 최대 체력 값 </param>
	/// <returns> : 잘못된 값이 들어왔을 경우 return false </returns>
	bool UpdateHPBar(float _HP, float _MaxHP);

	/// <summary>
	/// Boost Bar 업데이트
	/// </summary>
	/// <param name="_Boost"> : 현재 Boost 량 </param>
	/// <param name="_MaxBoost"> : 최대 Boost 량 </param>
	/// <returns> : 잘못된 값이 들어왔을 경우 return false </returns>
	bool UpdateBoostBar(float _Boost, float _MaxBoost);

private:
	
	
	/// <summary>
	/// 특정 ProgressBar 이번 Tick Lerp 처리 
	/// </summary>
	void LerpProgressBar(class UProgressBar* _TargetProgressBar, float _LerpAlphaSpeed, float _DestRatio, float _DeltaTime);	
	

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
	
};
