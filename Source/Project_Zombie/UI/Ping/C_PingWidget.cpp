// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PingWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameMode/C_UIManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/MainHUD/CompassBarWidget/C_CompassBarWidget.h"
#include "UI/MainHUD/CompassBarWidget/CompassMarkerWidget/C_CompassMarkerWidget.h"
#include "Utility/C_Util.h"

void UC_PingWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	m_MyPlayer = Cast<AC_BasicPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!m_MyPlayer) UC_Util::Print("From UC_PingWidget::NativeOnInitialized : m_MyPlayer init failed!", FColor::Red, 10.f);
	
}

void UC_PingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 시간 0으로 Animation 정지 처리(안보이게끔)
	PlayAnimation(SpawnAnimation);
	PauseAnimation(SpawnAnimation);
}

void UC_PingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (!m_bCurrentShowingPingMarker) return;
	
	// DistanceText 업데이트
	const float Distance      = FVector::Distance(m_SpawnedLocation, m_MyPlayer->GetActorLocation());
	const float DistanceMeter = Distance * 0.01f; // cm to meter
	const FString DistTextStr = FString::Printf(TEXT("%.1f m"), DistanceMeter);

	const FText DistText = FText::FromString(DistTextStr);
	
	DistanceText->SetText(DistText);
}

void UC_PingWidget::ShowPingWidget(const FVector& _WorldPingSpawnedLocation)
{
	// 이미 핑을 보여주는 상태
	if (m_bCurrentShowingPingMarker) return;

	m_bCurrentShowingPingMarker = true;
	m_SpawnedLocation           = _WorldPingSpawnedLocation;
	
	PlayAnimation(SpawnAnimation);
	
	if (APlayerController* PC = m_MyPlayer->GetController<APlayerController>())
	{
		if (AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD()))
		{
			m_TargetCompassMarkerWidget = UIManager->GetMainHUDWidget()->GetCompassBarWidget()->GetCompassMarkerWidget();
			m_TargetCompassMarkerWidget->TogglePingMarker(true);
			m_TargetCompassMarkerWidget->SetWorldMarkerSpawnedLocation(m_SpawnedLocation);
		}
	}
}

void UC_PingWidget::HidePingWidget()
{
	// 이미 핑을 보여주지 않는 상태
	if (!m_bCurrentShowingPingMarker) return;
	
	PlayAnimation(SpawnAnimation, 0.f, 1, EUMGSequencePlayMode::Reverse);
	m_TargetCompassMarkerWidget->TogglePingMarker(false);
	m_TargetCompassMarkerWidget = nullptr;
	m_bCurrentShowingPingMarker = false;
}
