// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CompassBarWidget.h"

#include "CompassMarkerWidget/C_CompassMarkerWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Utility/C_Util.h"
#include "WorldPartition/HLOD/HLODRuntimeSubsystem.h"

void UC_CompassBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	// Init CompassPingMarkers
	// 맥시멈 4명의 플레이어로 판단, CompassPingMarker 
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
	
	/* 맥시멈 3개의 GlobalPingMarker를 사용할 수 있도록 조치함 (TODO : 만약 이것보다 더 많은 글로벌 Marker 개수가 필요한 경우, 이 Pool 더 늘려주어야 함 */
	for (int i = 0; i < 3; ++i)
	{
		
		const FString CompassPingMarkerName = FString::Printf(TEXT("GlobalCompassPingMarker_%d"), i);
		const FName WidgetName(*CompassPingMarkerName);
		UC_CompassMarkerWidget* CompassPingMarker = Cast<UC_CompassMarkerWidget>(GetWidgetFromName(WidgetName));
		if (!CompassPingMarker)
		{
			UC_Util::Print("From UC_CompassBarWidget::NativeOnInitialized : CompassPingMarker init failed!", FColor::Red, 10.f);
			return;
		}
		m_GlobalCompassPingMarkers.Add(CompassPingMarker);		
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

bool UC_CompassBarWidget::SpawnGlobalPingMarker(EGamePingType _GamePingType, const FVector& _WorldPingLocation)
{
	for (UC_CompassMarkerWidget* GlobalPingMarker : m_GlobalCompassPingMarkers)
	{
		if (GlobalPingMarker->IsActive()) continue;
		
		GlobalPingMarker->TogglePingMarker(true, _GamePingType);
		GlobalPingMarker->SetWorldMarkerSpawnedLocation(_WorldPingLocation);
		return true;
	}

	// 사용 가능한 GlobalPingMarker가 없음 (TODO : GlobalPingMarker 풀을 더 늘려주어야 함)
	UC_Util::Print("[UC_CompassBarWidget::SpawnGlobalPingMarker] : Not enough pool count for Global CompassPingMarker", FColor::Red, 10.f);
	return false;
}

void UC_CompassBarWidget::HideAllGlobalPingMarkers()
{
	for (UC_CompassMarkerWidget* GlobalMarker : m_GlobalCompassPingMarkers)
		GlobalMarker->TogglePingMarker(false);
}
