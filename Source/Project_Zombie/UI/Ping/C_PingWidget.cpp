// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PingWidget.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/MainHUD/CompassBarWidget/C_CompassBarWidget.h"
#include "UI/MainHUD/CompassBarWidget/CompassMarkerWidget/C_CompassMarkerWidget.h"
#include "Utility/C_Util.h"

void UC_PingWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	// OnInitialized 시점에는 폰이 없을 수 있으므로 여기서는 캐싱을 시도만 합니다.
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		m_MyPlayer = Cast<AC_BasicPlayer>(PC->GetPawn());
	}
}

void UC_PingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 시간 0으로 Animation 정지 처리(안보이게끔)
	if (!m_bCurrentShowingPingMarker)
	{
		PlayAnimation(SpawnAnimation);
		PauseAnimation(SpawnAnimation);
	}
    
	// 1. 연쇄 참조 안전하게 쪼개기 (포인터 방어막 형성)
	if (m_OwnerPlayer)
	{
		AC_UIManager* UIManager = UI_MANAGER(GetWorld());
		if (UIManager)
		{
			UC_GameMainHUD* MainHUD = UIManager->GetMainHUDWidget();
			if (MainHUD)
			{
				UC_CompassBarWidget* CompassBar = MainHUD->GetCompassBarWidget();
				if (CompassBar)
				{
					m_TargetCompassMarkerWidget = CompassBar->RegisterPlayerCompassPingMarker(m_OwnerPlayer);
				}
			}
		}
	}
}

void UC_PingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (!m_bCurrentShowingPingMarker) return;
	
	// TODO : 타이머같은걸로 한번 세팅되면 끝나게 해야 좋을듯?
	// 2. 런타임에 m_MyPlayer가 널이라면 안전하게 다시 갱신 시도
	if (m_MyPlayer == nullptr)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			m_MyPlayer = Cast<AC_BasicPlayer>(PC->GetPawn());
		}
	}

	// 여전히 널이라면 이번 프레임은 연산을 건너뜁니다 (크래시 방지)
	if (m_MyPlayer == nullptr) return;
	
	// DistanceText 업데이트
	const float Distance      = FVector::Distance(m_SpawnedLocation, m_MyPlayer->GetActorLocation());
	const float DistanceMeter = Distance * 0.01f; // cm to meter
	const FString DistTextStr = FString::Printf(TEXT("%.1f m"), DistanceMeter);

	const FText DistText = FText::FromString(DistTextStr);
	
	DistanceText->SetText(DistText);
}

void UC_PingWidget::SetPingMarkerColor(const FColor& _Color) const
{
	PingMarkerImage->SetColorAndOpacity(_Color);
	
	if (m_TargetCompassMarkerWidget)
		m_TargetCompassMarkerWidget->SetPingMarkerColor(_Color);
}

void UC_PingWidget::ShowPingWidget(const FVector& _WorldPingSpawnedLocation, EGamePingType _PingType)
{
	// 이미 핑을 보여주는 상태
	if (m_bCurrentShowingPingMarker) return;

	// PingMarker 종류에 맞는 PingImage 세팅
	PingMarkerImage->SetBrushFromTexture(m_PingMarkerTextures[_PingType]);
	
	m_bCurrentShowingPingMarker = true;
	m_SpawnedLocation           = _WorldPingSpawnedLocation;
	
	PlayAnimation(SpawnAnimation);

	// 이 PingWidget에 대응되는 CompassMarkerWidget이 존재한다면, 해당 Widget 또한 활성화
	if (m_TargetCompassMarkerWidget)
	{
		m_TargetCompassMarkerWidget->TogglePingMarker(true, _PingType);
		m_TargetCompassMarkerWidget->SetWorldMarkerSpawnedLocation(m_SpawnedLocation);
	}
}

void UC_PingWidget::HidePingWidget()
{
	// 이미 핑을 보여주지 않는 상태
	if (!m_bCurrentShowingPingMarker) return;
	
	PlayAnimation(SpawnAnimation, 0.f, 1, EUMGSequencePlayMode::Reverse);

	// 이 PingWidget에 대응되는 CompassMarkerWidget이 존재한다면, 해당 Widget 또한 비활성화
	if (m_TargetCompassMarkerWidget)
		m_TargetCompassMarkerWidget->TogglePingMarker(false);
	
	m_bCurrentShowingPingMarker = false;
}
