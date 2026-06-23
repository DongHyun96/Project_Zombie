// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerStatWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Utility/C_Util.h"

void UC_PlayerStatWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	m_ShowAmmoInfosAnims = 
	{
		{EFireMode::Single,		ShowAmmoInfos_SingleMode},
		{EFireMode::Burst,		ShowAmmoInfos_BurstMode},
		{EFireMode::FullAuto,	ShowAmmoInfos_AutoMode}
	};
	
	m_MagazineTexts = { MagazineText1, MagazineText2 };
	m_LeftAmmoTexts = { LeftAmmoText1, LeftAmmoText2 }; 
	
	m_UpdateMagazineTextAnimations = { UpdateMagazineText1, UpdateMagazineText2 };
	m_UpdateTotalLeftAmmoAnimations = { UpdateLeftAmmo1, UpdateLeftAmmo2 };
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

bool UC_PlayerStatWidget::ToggleAmmoInfoVisibility
(
	bool		_Visible,
	EFireMode	_FireMode,
	int32 	 	_MagazineAmmo,
	int32 	 	_LeftAmmoTotalCount
)
{
	if (!_Visible)
	{
		// 이전까지 보여줬던 FireMode 전용 ShowAnimation 역으로 재생처리
		PlayAnimation(m_ShowAmmoInfosAnims[m_CurrentShowingFireMode], 0.f, 1, EUMGSequencePlayMode::Reverse);
		return true;
	}


	if (_FireMode == EFireMode::End)
	{
		UC_Util::Print("From UC_PlayerStatWidget::ToggleAmmoInfoVisibility : Use valid FireMode param", FColor::Red, 10.f);
		return false;
	}
	m_CurrentShowingFireMode = _FireMode;

	m_bCurrentShowingMagTextIdx   = false; // IDX 0
	m_bCurrentShowingLeftAmmoText = false;

	// 들어온 총알 값 세팅
	SetMagazineText(_MagazineAmmo);
	SetLeftAmmoText(_LeftAmmoTotalCount);

	// 현재의 FireMode에 맞는 ShowingAmmoInfoAnim 재생
	PlayAnimation(m_ShowAmmoInfosAnims[m_CurrentShowingFireMode]);
	
	return true;
}

void UC_PlayerStatWidget::UpdateMagazineAmmoCount(int32 _AmmoCount)
{
	// 다음으로 보여줄 Text로 지속적으로 Swapping
	m_bCurrentShowingMagTextIdx = !m_bCurrentShowingMagTextIdx;

	// 실질적인 장탄 수 입력
	SetMagazineText(_AmmoCount);
	
	// Animation 재생
	PlayAnimation(m_UpdateMagazineTextAnimations[static_cast<int32>(m_bCurrentShowingMagTextIdx)]);
}

void UC_PlayerStatWidget::UpdateLeftAmmoTotalCount(int32 _LeftAmmoTotalCount)
{
	// 다음으로 보여줄 Text로 지속적으로 Swapping
	m_bCurrentShowingLeftAmmoText = !m_bCurrentShowingLeftAmmoText;

	// 실질적인 전체 탄수 정보 세팅
	SetLeftAmmoText(_LeftAmmoTotalCount);
	
	// Animation 재생
	PlayAnimation(m_UpdateTotalLeftAmmoAnimations[static_cast<int32>(m_bCurrentShowingLeftAmmoText)]);
}

void UC_PlayerStatWidget::LerpProgressBar(UProgressBar* _TargetProgressBar, float _LerpAlphaSpeed, float _DestRatio, float _DeltaTime)
{
	float Ratio = _TargetProgressBar->GetPercent();
	Ratio = FMath::Lerp(Ratio, _DestRatio, _DeltaTime * _LerpAlphaSpeed);
	_TargetProgressBar->SetPercent(Ratio);
}

void UC_PlayerStatWidget::SetMagazineText(int32 _AmmoAmount)
{
	static FLinearColor MAG_NORMAL_COLOR = { 0.71875f, 0.71875f, 0.71875f, 1.f };

	UTextBlock* TargetTextBlock = m_MagazineTexts[static_cast<int32>(m_bCurrentShowingMagTextIdx)];
	
	TargetTextBlock->SetText(FText::AsNumber(_AmmoAmount));
	
	// Ammo Amount가 0이라면, Color Red로 처리
	TargetTextBlock->SetColorAndOpacity(_AmmoAmount > 0 ? FSlateColor(MAG_NORMAL_COLOR) : FSlateColor(FLinearColor::Red));
}

void UC_PlayerStatWidget::SetLeftAmmoText(int32 _AmmoAmount)
{
	const FString DisplayString = FString(TEXT("/ ")) + FString::FromInt(_AmmoAmount);
	
	// m_LeftAmmoTexts[static_cast<int32>(m_bCurrentShowingLeftAmmoText)]->SetText(FText::AsNumber(_AmmoAmount));
	m_LeftAmmoTexts[static_cast<int32>(m_bCurrentShowingLeftAmmoText)]->SetText(FText::FromString(std::move(DisplayString)));
}

