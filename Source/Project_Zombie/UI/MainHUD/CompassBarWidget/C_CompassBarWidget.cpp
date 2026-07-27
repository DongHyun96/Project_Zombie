// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CompassBarWidget.h"

#include "CompassMarkerWidget/C_CompassMarkerWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Utility/C_Util.h"

void UC_CompassBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	// Init CompassPingMarkers
	for (int i = 0; i < 4; ++i)
	{
		const FString CompassPingMarkerName = FString::Printf(TEXT("CompassPingMarker_%d"), i);
		const FName WidgetName(*CompassPingMarkerName);
		UC_CompassMarkerWidget* CompassPingMarker = Cast<UC_CompassMarkerWidget>(GetWidgetFromName(WidgetName));
		if (!CompassPingMarker)
		{
			UC_Util::Print("From UC_CompassBarWidget::NativeOnInitialized : CompassPingMarker init failed!", FColor::Red, 10.f);
			return;
		}
		m_CompassPingMarkerPool.Add(CompassPingMarker);
	}
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

	const float PCYaw        = PlayerController->GetControlRotation().Yaw;
	const float MtrlYawValue = PCYaw / 360.f;
	
	m_CompassBarDynamicMtrl->SetScalarParameterValue(TEXT("yaw"), MtrlYawValue);

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

UC_CompassMarkerWidget* UC_CompassBarWidget::RegisterPlayerCompassPingMarker(AC_BasicPlayer* _Player)
{
	if (!_Player) return nullptr;
	
	if (m_CompassPingMarkerPool.IsEmpty())
	{
		UC_Util::Print("From UC_CompassBarWidget::RegisterPlayerCompassPingMarker : Pool count 0", FColor::Red, 10.f);
		return nullptr;
	}

	UC_CompassMarkerWidget* TargetMarker = m_CompassPingMarkerPool.Pop();
	if (!TargetMarker)
	{
		UC_Util::Print("From UC_CompassBarWidget::RegisterPlayerCompassPingMarker : TargetMarker nullptr", FColor::Red, 10.f);
		return nullptr;
	}

	m_mapCompassPingMarkers.Add(_Player, TargetMarker);
	return TargetMarker;
}
