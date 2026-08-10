// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerStatWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_PlayerStatComponent.h"
#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameModeAndManager/C_UIManager.h"
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

void UC_PlayerStatWidget::ToggleBoostBarColor(bool BoostExhausted)
{
	if (BoostExhausted)
	{
		FProgressBarStyle CurrentStyle = BoostBar->WidgetStyle;

		CurrentStyle.FillImage.TintColor = FSlateColor(FLinearColor(1, 0.02, 0, 1));

		BoostBar->SetWidgetStyle(CurrentStyle);
	}
	else
	{
		FProgressBarStyle CurrentStyle = BoostBar->WidgetStyle;

		CurrentStyle.FillImage.TintColor = FSlateColor(FLinearColor(1, 0.534041, 0, 1));

		BoostBar->SetWidgetStyle(CurrentStyle);
	}
}

void UC_PlayerStatWidget::BindCurHPUpdate(UC_StatComponentBase* InPlayerStatComponent)
{
	
	UC_PlayerStatComponent* PlayerStatComp= Cast<UC_PlayerStatComponent>(InPlayerStatComponent);
	
	if (!PlayerStatComp) return;
	
	AC_BasicPlayer* OwnerPlayer = Cast<AC_BasicPlayer>(InPlayerStatComponent->GetOwnerCharacter());
	
	if (!OwnerPlayer) return;
	
	if (OwnerPlayer->IsLocallyControlled())
	{
		
			//AC_UIManager* UIManager = UI_MANAGER(GetWorld());
			//if (!UIManager) return;
			//
			//UC_GameMainHUD* MainHUD = UIManager->GetMainHUDWidget();
			//if (!MainHUD) return;
			//
			//UC_PlayerStatWidget* StatWidget = MainHUD->GetPlayerStatWidget();
			//if (StatWidget)
		PlayerStatComp->OnCurHPUpdatedDelegate.AddUObject(this, &UC_PlayerStatWidget::UpdateHPBarRatio);
		
		UpdateHPBarRatio(PlayerStatComp->GetCurHPRatio());
	}
	// else PlayerStatComp->BindUpdateOtherPlayerHPBar();
}

bool UC_PlayerStatWidget::UpdateHPBar(float _HP, float _MaxHP)
{
	// 0 나누기	방어 및 MaxHP 값보다 현재 체력이 크다고 Input이 들어온 상황
	if (_MaxHP <= 0.f || _HP > _MaxHP)
	{
		UC_Util::Print("From UC_PlayerStatWidget::UpdateHPBar : Invalid Param received!", FColor::Red, 5.f);
		return false;
	}

	const float NewRatio  = _HP / _MaxHP;
	
	UpdateHPBarBoilerPlate(NewRatio);
	return true;
}

void UC_PlayerStatWidget::UpdateHPBarRatio(float _Ratio)
{
	if (_Ratio < 0.f || _Ratio > 1.f)
	{
		UC_Util::Print("From UC_PlayerStatWidget::UpdateHPBar : Invalid Param received!", FColor::Red, 5.f);
		return;
	}
	
	UpdateHPBarBoilerPlate(_Ratio);
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

void UC_PlayerStatWidget::UpdateHPBarBoilerPlate(float _Ratio)
{
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
		if (!m_bAmmoInfoPlayedReverseFlag) // 역재생 Animation 처리로 마지막에 호출하지 않았을 때 역재생 새로이 재생
		{
			// 이전까지 보여줬던 FireMode 전용 ShowAnimation 역으로 재생처리
			// Idx 0번을 메인으로 띄워줘야 함 -> 이미 Idx 0번이면 처리할 필요 x
			// 1번을 보여주던 상태에서 역재생으로 보이지 않게끔 처리할 때, 0번 Idx로 맞추고 동시에 내용도 1번 Idx 내용물로 바꿔야한다
			if (m_bCurrentShowingMagTextIdx)
			{
				PasteCurrentShowingMagTextToHidden(); // 내용물 현재 보여지는 내용물로 세팅
				m_bCurrentShowingMagTextIdx = false; // idx 0번으로 세팅
			}
			
			if (m_bCurrentShowingLeftAmmoTextIdx)
			{
				PasteCurrentShowingLeftAmmoTextToHidden(); // 내용물 현재 보여지는 내용물로 세팅
				m_bCurrentShowingLeftAmmoTextIdx = false; // idx 0번으로 세팅
			}
			
			PlayAnimation(m_ShowAmmoInfosAnims[m_CurrentShowingFireMode], 0.f, 1, EUMGSequencePlayMode::Reverse);
			m_bAmmoInfoPlayedReverseFlag = true;
		}
		return true;
	}

	if (_FireMode == EFireMode::End)
	{
		UC_Util::Print("From UC_PlayerStatWidget::ToggleAmmoInfoVisibility : Use valid FireMode param", FColor::Red, 10.f);
		return false;
	}
	
	if (!m_bAmmoInfoPlayedReverseFlag) // 이미 AmmoInfo가 화면에 보이고 있는 상태에서 재호출된 경우
	{
		// FireMode가 달라졌다면 모드 전환 애니메이션 실행
		if (m_CurrentShowingFireMode != _FireMode)
			UpdateFireMode(_FireMode);

		// 2. 진행 중이던 Text 스왑 애니메이션이 있다면 즉시 중지 (위치/알파 꼬임 방지)
		for (UWidgetAnimation* Anim : m_UpdateMagazineTextAnimations)
			if (Anim && IsAnimationPlaying(Anim)) StopAnimation(Anim);
		
		for (UWidgetAnimation* Anim : m_UpdateTotalLeftAmmoAnimations)
			if (Anim && IsAnimationPlaying(Anim)) StopAnimation(Anim);

		// 인덱스를 0으로 강제 초기화하지 않고, 현재 보여지는 TextBlock에 즉시 값 적용
		SetCurrentShowingMagazineText(_MagazineAmmo);
		SetCurrentShowingLeftAmmoText(_LeftAmmoTotalCount);

		// 안 보이는 Hidden Text도 현재 값으로 동일하게 맞춰둠
		PasteCurrentShowingMagTextToHidden();
		PasteCurrentShowingLeftAmmoTextToHidden();

		return true;
	}

	// 최초 AmmoInfo Visible true 
	m_CurrentShowingFireMode = _FireMode;

	m_bCurrentShowingMagTextIdx      = false; // IDX 0 시작
	m_bCurrentShowingLeftAmmoTextIdx = false;

	SetCurrentShowingMagazineText(_MagazineAmmo);
	SetCurrentShowingLeftAmmoText(_LeftAmmoTotalCount);

	m_bAmmoInfoPlayedReverseFlag = false;
	PlayAnimation(m_ShowAmmoInfosAnims[m_CurrentShowingFireMode]);
    
	return true;
}

void UC_PlayerStatWidget::UpdateMagazineAmmoCount(int32 _AmmoCount)
{
	// 현재 Ammo Info를 보여주고 있지 않은 상황
	if (m_bAmmoInfoPlayedReverseFlag) return;

	// UI가 등장하는 애니메이션이 아직 재생 중인지 체크
	if (IsAnimationPlaying(m_ShowAmmoInfosAnims[m_CurrentShowingFireMode]))
	{
		// 애니메이션 없이 값만 즉시 덮어씌움
		SetCurrentShowingMagazineText(_AmmoCount);
		return;
	}
	
	// 다음으로 보여줄 Text로 지속적으로 Swapping
	m_bCurrentShowingMagTextIdx = !m_bCurrentShowingMagTextIdx;

	// 실질적인 장탄 수 입력
	SetCurrentShowingMagazineText(_AmmoCount);
	
	// Animation 재생
	PlayAnimation(m_UpdateMagazineTextAnimations[static_cast<int32>(m_bCurrentShowingMagTextIdx)]);
}

void UC_PlayerStatWidget::UpdateLeftAmmoTotalCount(int32 _LeftAmmoTotalCount)
{
	// 현재 Ammo Info를 보여주고 있지 않은 상황
	if (m_bAmmoInfoPlayedReverseFlag) return;
	
	// 다음으로 보여줄 Text로 지속적으로 Swapping
	m_bCurrentShowingLeftAmmoTextIdx = !m_bCurrentShowingLeftAmmoTextIdx;

	// 실질적인 전체 탄수 정보 세팅
	SetCurrentShowingLeftAmmoText(_LeftAmmoTotalCount);
	
	// Animation 재생
	PlayAnimation(m_UpdateTotalLeftAmmoAnimations[static_cast<int32>(m_bCurrentShowingLeftAmmoTextIdx)]);
}

bool UC_PlayerStatWidget::UpdateFireMode(EFireMode _NewFireMode)
{
	// AmmoInfo Visibility 비활성화 상태
	if (m_bAmmoInfoPlayedReverseFlag) return false;
	if (m_CurrentShowingFireMode == _NewFireMode) return false;
	
	switch (m_CurrentShowingFireMode)
	{
	case EFireMode::Single:
		if (_NewFireMode == EFireMode::Burst)	PlayAnimation(SingleToBurst);
		else									PlayAnimation(SingleToAuto);
		break;
	case EFireMode::Burst:
		if (_NewFireMode == EFireMode::Single)	PlayAnimation(BurstToSingle);
		else									PlayAnimation(BurstToAuto);
		break;
	case EFireMode::FullAuto:
		if (_NewFireMode == EFireMode::Single)	PlayAnimation(AutoToSingle);
		else									PlayAnimation(AutoToBurst);
		break;
	case EFireMode::End: return false;
	}

	m_CurrentShowingFireMode = _NewFireMode;
	return true;
}

void UC_PlayerStatWidget::LerpProgressBar(UProgressBar* _TargetProgressBar, float _LerpAlphaSpeed, float _DestRatio, float _DeltaTime)
{
	float Ratio = _TargetProgressBar->GetPercent();
	Ratio = FMath::Lerp(Ratio, _DestRatio, _DeltaTime * _LerpAlphaSpeed);
	_TargetProgressBar->SetPercent(Ratio);
}

void UC_PlayerStatWidget::SetCurrentShowingMagazineText(int32 _AmmoAmount)
{
	static FLinearColor MAG_NORMAL_COLOR = { 0.71875f, 0.71875f, 0.71875f, 1.f };

	UTextBlock* TargetTextBlock = m_MagazineTexts[static_cast<int32>(m_bCurrentShowingMagTextIdx)];
	
	TargetTextBlock->SetText(FText::AsNumber(_AmmoAmount));
	
	// Ammo Amount가 0이라면, Color Red로 처리
	TargetTextBlock->SetColorAndOpacity(_AmmoAmount > 0 ? FSlateColor(MAG_NORMAL_COLOR) : FSlateColor(FLinearColor::Red));
}

void UC_PlayerStatWidget::SetCurrentShowingLeftAmmoText(int32 _AmmoAmount)
{
	const FString DisplayString = FString(TEXT("/ ")) + FString::FromInt(_AmmoAmount);
	
	// m_LeftAmmoTexts[static_cast<int32>(m_bCurrentShowingLeftAmmoText)]->SetText(FText::AsNumber(_AmmoAmount));
	m_LeftAmmoTexts[static_cast<int32>(m_bCurrentShowingLeftAmmoTextIdx)]->SetText(FText::FromString(std::move(DisplayString)));
}

void UC_PlayerStatWidget::PasteCurrentShowingMagTextToHidden()
{
	UTextBlock* HiddenTextBlock  = m_MagazineTexts[static_cast<int32>(!m_bCurrentShowingMagTextIdx)];
	UTextBlock* ShowingTextBlock = m_MagazineTexts[static_cast<int32>(m_bCurrentShowingMagTextIdx)];
	
	HiddenTextBlock->SetText(ShowingTextBlock->GetText());
}

void UC_PlayerStatWidget::PasteCurrentShowingLeftAmmoTextToHidden()
{
	UTextBlock* HiddenTextBlock  = m_LeftAmmoTexts[static_cast<int32>(!m_bCurrentShowingLeftAmmoTextIdx)];
	UTextBlock* ShowingTextBlock = m_LeftAmmoTexts[static_cast<int32>(m_bCurrentShowingLeftAmmoTextIdx)];
	
	HiddenTextBlock->SetText(ShowingTextBlock->GetText());
}

