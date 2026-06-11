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
	const int32 DegreeValue    = FMath::RoundToInt(FRotator::ClampAxis(_RotationYaw) / 5.f) * 5 % 360;
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
