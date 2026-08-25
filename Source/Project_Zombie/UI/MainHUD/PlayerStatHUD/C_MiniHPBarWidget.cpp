// Fill out your copyright notice in the Description page of Project Settings.


#include "C_MiniHPBarWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/PlayerProfileComponent/C_PlayerProfileComponent.h"
#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Utility/C_Util.h"

void UC_MiniHPBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UC_MiniHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UC_MiniHPBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (m_bEnableDamageBarLerp)
		LerpProgressBar(DamageIndicatorBar, 5.f, m_DamageIndicatorPercentLerpDest, InDeltaTime);
	
	// Main HP Bar Lerp 처리
	LerpProgressBar(MainHPBar, 12.f, m_MainHPBarPercentLerpDest, InDeltaTime);
}

void UC_MiniHPBarWidget::Activate(AC_BasicPlayer* _TargetPlayer)
{
	if (!_TargetPlayer)
	{
		UC_Util::Print("[UC_MiniHPBarWidget::Activate] : _TargetPlayer nullptr", FColor::Red, 10.f);
		return;
	}

	if (!_TargetPlayer->GetPlayerState())
	{
		UC_Util::Print("[UC_MiniHPBarWidget::Activate] : _TargetPlayer's PlayerState nullptr", FColor::Red, 10.f);
		return;
	}
	
	const FString& PlayerName = _TargetPlayer->GetPlayerState()->GetPlayerName();
	const FColor& PlayerColor = _TargetPlayer->GetPlayerProfileComponent()->GetPlayerSelectedColor();

	PlayerColorImage->SetColorAndOpacity(PlayerColor);
	PlayerNameText->SetText(FText::FromString(PlayerName.IsEmpty() ? "Anonymous" : PlayerName));
	
	// Activate와 동시에 현 HP Ratio 갱신 처리 또한 해준다
	UpdateHPBar(_TargetPlayer->GetStatComponent()->GetCurHPRatio());
	
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UC_MiniHPBarWidget::UpdateHPBar(float _Ratio)
{
	if (_Ratio < 0.f || _Ratio > 1.f)
	{
		UC_Util::Print("From UC_MiniHPBarWidget::UpdateHPBar : Invalid Param received!", FColor::Red, 5.f);
		return;
	}
	
	// 힐 적용인지 Damage 입는지에 따라 처리
	const float PrevRatio = MainHPBar->GetPercent();
	
	// MainHPbar의 경우, 힐 적용이 되었든, Damage 처리가 되었든 LerpDest 적용은 동일
	m_MainHPBarPercentLerpDest = _Ratio;
	
	if (_Ratio > PrevRatio) // 힐 적용
	{
		// Damage Bar 관련 처리
		DamageIndicatorBar->SetPercent(0.f);     // 아예 DamageBar 안보이게끔 처리
		m_bEnableDamageBarLerp = false;   // DamageBar Lerp 적용 끄기
	}
	else // Damage를 입었을 때
	{
		DamageIndicatorBar->SetPercent(PrevRatio); // 이전 HPBar의 Ratio부터 시작해서 DamageBar Lerp 처리 시작
		m_bEnableDamageBarLerp = true;
		m_DamageIndicatorPercentLerpDest = _Ratio;
	}
}

void UC_MiniHPBarWidget::LerpProgressBar(UProgressBar* _TargetProgressBar, float _LerpAlphaSpeed, float _DestRatio, float _DeltaTime)
{
	float Ratio = _TargetProgressBar->GetPercent();
	Ratio = FMath::Lerp(Ratio, _DestRatio, _DeltaTime * _LerpAlphaSpeed);
	_TargetProgressBar->SetPercent(Ratio);
}


