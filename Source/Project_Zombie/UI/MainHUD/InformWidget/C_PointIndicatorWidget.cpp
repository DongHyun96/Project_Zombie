// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PointIndicatorWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/PointTower/C_PointTower.h"
#include "Components/Image.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "Utility/C_Util.h"

void UC_PointIndicatorWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UC_PointIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UC_PointIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (!m_TargetPointTower) return;

	// Lazy init
	if (!m_LocalMainPlayer)
	{
		if (!LEVEL_MANAGER) return;
		m_LocalMainPlayer = LEVEL_MANAGER->GetLocalPlayer();
	}
	
	if (!m_LocalMainPlayer)
	{
		PRINT_LOCAL(GetWorld(), "[UC_PointIndicatorWidget::NativeTick] : Lazy init failed", FColor::Red, 10.f);
		return;
	}

	/* 높낮이 표시기 업데이트 처리 */
	
	// LocalMainPlayer와 TargetPointTower의 Generator와의 높이차를 구함
	const float HeightDiff = m_TargetPointTower->GetGenerator()->GetComponentLocation().Z - m_LocalMainPlayer->GetActorLocation().Z;
	
	// 2.5m 더 높게 Generator가 있다면, Above로 판단 | 낮다면, Below로 판단
	EPointIndicatorAltitudeType CurAltitudeType = (HeightDiff > 250.f) ?  EPointIndicatorAltitudeType::Above :
												  (HeightDiff < -250.f) ? EPointIndicatorAltitudeType::Below : EPointIndicatorAltitudeType::Normal;

	// 실질적인 상태값이 변경되었을 경우에만 Visibility 업데이트 처리를 한다
	if (CurAltitudeType == m_AltitudeType) return;
	m_AltitudeType = CurAltitudeType;
	
	switch (m_AltitudeType)
	{
	case EPointIndicatorAltitudeType::Normal:
		UpArrowImage->SetVisibility(ESlateVisibility::Collapsed);
		DownArrowImage->SetVisibility(ESlateVisibility::Collapsed);
		break;
	case EPointIndicatorAltitudeType::Above:
		UpArrowImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		DownArrowImage->SetVisibility(ESlateVisibility::Collapsed);
		break;
	case EPointIndicatorAltitudeType::Below:
		UpArrowImage->SetVisibility(ESlateVisibility::Collapsed);
		DownArrowImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		break;
	}
		
}

void UC_PointIndicatorWidget::SetPointTower(AC_PointTower* _PointTower)
{
	m_TargetPointTower = _PointTower;
	SetVisibility(m_TargetPointTower == nullptr ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
}

void UC_PointIndicatorWidget::SetVisibility(ESlateVisibility InVisibility)
{
	Super::SetVisibility(InVisibility);
	
	if (InVisibility == ESlateVisibility::Collapsed || InVisibility == ESlateVisibility::Hidden) m_bVisible = false;
	else m_bVisible = true;
}

void UC_PointIndicatorWidget::SetTriangleImgAngle(float _Angle)
{
	TriangleImage->SetRenderTransformAngle(_Angle);
}
