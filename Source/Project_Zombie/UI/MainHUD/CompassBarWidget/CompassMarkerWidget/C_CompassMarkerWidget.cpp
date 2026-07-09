// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CompassMarkerWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utility/C_Util.h"

void UC_CompassMarkerWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	m_MarkerDynamicMtrl     = CompassMarkerImage->GetDynamicMaterial();
	m_LocalPlayer           = Cast<AC_BasicPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	m_LocalPlayerController = m_LocalPlayer->GetController<APlayerController>();
	
	if (!m_LocalPlayer)
		UC_Util::Print("From UC_CompassMarkerWidget::NativeOnInitialized : LocalPlayer init failed!", FColor::Red, 10.f);
}

void UC_CompassMarkerWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UC_CompassMarkerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// MeterText 값 수정 및 위치(yaw값에 따른)조정 처리
	if (!m_bIsActive) return;

	/* Update Mtrl Yaw */
	const float RawControllerYaw = m_LocalPlayerController->GetControlRotation().Yaw;
	const float RawLookAtYaw     = UKismetMathLibrary::FindLookAtRotation(m_LocalPlayer->GetActorLocation(), m_WorldMarkerSpawnedLocation).Yaw;
	
    
	m_MarkerDynamicMtrl->SetScalarParameterValue(TEXT("yaw"), (RawControllerYaw - RawLookAtYaw) / 360.f);

	/* DistanceText 업데이트 */
	const float Distance      = FVector::Distance(m_WorldMarkerSpawnedLocation, m_LocalPlayer->GetActorLocation());
	const float DistanceMeter = Distance * 0.01f; // cm to meter
	const FString DistTextStr = FString::Printf(TEXT("%.1f m"), DistanceMeter);

	const FText DistText = FText::FromString(DistTextStr);
	
	PingMarkerMeterText->SetText(DistText);

	/* DistanceText 위치 업데이트 */
	// 0도 ~ 360 -> -180 ~ 180
	float DeltaYaw = RawLookAtYaw - RawControllerYaw;
	DeltaYaw = FRotator::NormalizeAxis(DeltaYaw);

	static const float CompassHalfFOV = 210.f * 0.5f;   // 나침반 UI가 화면에 보여줄 총 각도 범위 (예: 좌우 90도씩 총 180도)

	// 마커가 나침반 가시 범위 안에 있을 때만 위치를 계산하고 표시합니다.
	if (FMath::Abs(DeltaYaw) > CompassHalfFOV)
	{
		if (PingMarkerMeterText->GetVisibility() != ESlateVisibility::Collapsed)
			PingMarkerMeterText->SetVisibility(ESlateVisibility::Collapsed);
		
		return;
	}
	
	const float TransX = FMath::GetMappedRangeValueClamped(FVector2D(-CompassHalfFOV, CompassHalfFOV), FVector2D(-m_CompassHalfWidth, m_CompassHalfWidth), DeltaYaw);
	
	// Render Transform 적용 (텍스트의 기준 피벗이 Center여야 중심에 맞게 움직입니다)
	FWidgetTransform WidgetTransform = PingMarkerMeterText->GetRenderTransform();
	WidgetTransform.Translation.X = TransX;
	PingMarkerMeterText->SetRenderTransform(WidgetTransform);

	// 가시 범위 안이므로 보이게 설정
	if (PingMarkerMeterText->GetVisibility() != ESlateVisibility::HitTestInvisible)
		PingMarkerMeterText->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UC_CompassMarkerWidget::TogglePingMarker(bool _Visible)
{
	m_bIsActive = _Visible;
	CompassMarkerImage->SetVisibility(_Visible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	PingMarkerMeterText->SetVisibility(_Visible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}
