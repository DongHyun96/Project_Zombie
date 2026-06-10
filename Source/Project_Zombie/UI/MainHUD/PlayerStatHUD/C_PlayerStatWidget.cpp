// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerStatWidget.h"

#include "Components/ProgressBar.h"
#include "Utility/C_Util.h"

void UC_PlayerStatWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UC_PlayerStatWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UC_PlayerStatWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (m_bEnableDamageBarLerp)
		LerpProgressBar(DamageIndicatorBar, 5.f, m_DamageIndicatorPercentLerpDest, InDeltaTime);
	
	// Main HP Bar Lerp 처리
	LerpProgressBar(MainHPBar, 12.f, m_MainHPBarPercentLerpDest, InDeltaTime);
	
	// Boost Bar Lerp 처리
	LerpProgressBar(BoostBar, 12.f, m_BoostBarPercentLerpDest, InDeltaTime);
}

bool UC_PlayerStatWidget::UpdateHPBar(float _HP, float _MaxHP)
{
	// 0 나누기	방어 및 MaxHP 값보다 현재 체력이 크다고 Input이 들어온 상황
	if (_MaxHP <= 0.f || _HP > _MaxHP)
	{
		UC_Util::Print("From UC_PlayerStatWidget::UpdateHPBar : Invalid Param received!", FColor::Red, 5.f);
		return false;
	}

	// 힐 적용인지 Damage 입는지에 따라 처리
	const float PrevRatio = MainHPBar->GetPercent();
	const float NewRatio  = _HP / _MaxHP;
	
	// MainHPbar의 경우, 힐 적용이 되었든, Damage 처리가 되었든 LerpDest 적용은 동일
	m_MainHPBarPercentLerpDest = NewRatio;
	
	if (NewRatio > PrevRatio) // 힐 적용
	{
		// Damage Bar 관련 처리
		DamageIndicatorBar->SetPercent(0.f);     // 아예 DamageBar 안보이게끔 처리
		m_bEnableDamageBarLerp = false;   // DamageBar Lerp 적용 끄기
	}
	else // Damage를 입었을 때
	{
		DamageIndicatorBar->SetPercent(PrevRatio); // 이전 HPBar의 Ratio부터 시작해서 DamageBar Lerp 처리 시작
		m_bEnableDamageBarLerp = true;
		m_DamageIndicatorPercentLerpDest = NewRatio;
	}
	
	return true;
}

bool UC_PlayerStatWidget::UpdateBoostBar(float _Boost, float _MaxBoost)
{
	if (_MaxBoost <= 0.f || _Boost > _MaxBoost)
	{
		UC_Util::Print("From UC_PlayerStatWidget::UpdateBoostBar : Invalid Param received!", FColor::Red, 5.f);
		return false;
	}
		
	m_BoostBarPercentLerpDest = _Boost / _MaxBoost;
	return true;
}

void UC_PlayerStatWidget::LerpProgressBar(UProgressBar* _TargetProgressBar, float _LerpAlphaSpeed, float _DestRatio, float _DeltaTime)
{
	float Ratio = _TargetProgressBar->GetPercent();
	Ratio = FMath::Lerp(Ratio, _DestRatio, _DeltaTime * _LerpAlphaSpeed);
	_TargetProgressBar->SetPercent(Ratio);
}

