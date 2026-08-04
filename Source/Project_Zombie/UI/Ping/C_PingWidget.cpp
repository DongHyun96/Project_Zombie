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
	
	m_MyPlayer = Cast<AC_BasicPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!m_MyPlayer) UC_Util::Print("From UC_PingWidget::NativeOnInitialized : m_MyPlayer init failed!", FColor::Red, 10.f);
	
}

void UC_PingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UC_Util::Print("[UC_PingWidget::NativeConstruct]", FColor::Green, 10.f);
	
	// 시간 0으로 Animation 정지 처리(안보이게끔)
	// NativeConstruct 호출되기 이전, 이미 보이게끔 처리를 했을 수 있음
	if (!m_bCurrentShowingPingMarker)
	{
		PlayAnimation(SpawnAnimation);
		PauseAnimation(SpawnAnimation);
	}
	
	// m_OwnerPlayer가 제대로 잡힌 PingWidget의 경우(Player의 Ping), CompassBarWidget에 대응되는 PingMarker 등록
	if (m_OwnerPlayer)
		m_TargetCompassMarkerWidget = UI_MANAGER(GetWorld())->GetMainHUDWidget()->GetCompassBarWidget()->RegisterPlayerCompassPingMarker(m_OwnerPlayer);
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
