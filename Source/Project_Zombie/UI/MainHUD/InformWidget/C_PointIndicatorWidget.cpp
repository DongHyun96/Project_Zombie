// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PointIndicatorWidget.h"

#include "Components/Image.h"

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

	// TODO : Viewport에 PointTower가 잡히지 않을 경우, PointTower 방면 Rotation으로 삼각형 회전 맞춰줄 것
	
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
