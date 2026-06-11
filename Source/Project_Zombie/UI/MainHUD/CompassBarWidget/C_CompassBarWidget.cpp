// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CompassBarWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Utility/C_Util.h"

void UC_CompassBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UC_CompassBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!CompassBarImage)
	{
		UC_Util::Print("From UC_CompassBarWidget::NativeConstruct : CompassBarImage not bound", FColor::Red, 10.f);
		return;
	}
	
	m_CompassBarDynamicMtrl = CompassBarImage->GetDynamicMaterial();
}

void UC_CompassBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController) return;

	const float PCYaw = PlayerController->GetControlRotation().Yaw;
	
	m_CompassBarDynamicMtrl->SetScalarParameterValue(TEXT("yaw"), PCYaw / 360.f);
	SetDisplayDegreeText(PCYaw);
}

void UC_CompassBarWidget::SetDisplayDegreeText(float _RotationYaw)
{
	/*const float Yaw = (_RotationYaw < 0.f) ? _RotationYaw + 360.f : _RotationYaw;
	int32 DegreeValue{};
	
	if (Yaw >= 357.5) DegreeValue = 0;
	else
	{
		const int32 YawInt  = Yaw + 2.5f;
		const float ToFloat = YawInt / 5.f;
		DegreeValue         = ToFloat * 5;
	}
	
	if (m_DegreeToAlphabet.Contains(DegreeValue))
	{
		DegIndicatorText->SetText(m_DegreeToAlphabet[DegreeValue]);
		DegIndicatorText->SetColorAndOpacity
		(
			DegreeValue == 0 ? FSlateColor(m_ZeroDegColor) : FSlateColor(FLinearColor::White) 
		);
	}
	else
	{
		DegIndicatorText->SetText(FText::AsNumber(DegreeValue));
		DegIndicatorText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}*/
	
	/*// -180~180 -> 0~360 변환
	const float Yaw = (_RotationYaw < 0.f)
		? _RotationYaw + 360.f
		: _RotationYaw;

	// 가장 가까운 5도 단위로 반올림
	int32 DegreeValue = FMath::RoundToInt(Yaw / 5.f) * 5;

	// 360도는 0도로 처리
	if (DegreeValue >= 360)
	{
		DegreeValue = 0;
	}

	if (const FText* DirectionText = m_DegreeToAlphabet.Find(DegreeValue))
	{
		DegIndicatorText->SetText(*DirectionText);

		DegIndicatorText->SetColorAndOpacity(
			DegreeValue == 0
				? FSlateColor(m_ZeroDegColor)
				: FSlateColor(FLinearColor::White)
		);
	}
	else
	{
		DegIndicatorText->SetText(FText::AsNumber(DegreeValue));
		DegIndicatorText->SetColorAndOpacity(
			FSlateColor(FLinearColor::White)
		);
	}*/
	
	const int32 DegreeValue = FMath::RoundToInt(FRotator::ClampAxis(_RotationYaw) / 5.f) * 5 % 360;

	const FText* DirectionText = m_DegreeToAlphabet.Find(DegreeValue);

	DegIndicatorText->SetText
	(
		DirectionText != nullptr ? *DirectionText : FText::AsNumber(DegreeValue)
	);

	DegIndicatorText->SetColorAndOpacity
	(
		FSlateColor( DegreeValue == 0 ? m_ZeroDegColor : FLinearColor::White)
	);
}
