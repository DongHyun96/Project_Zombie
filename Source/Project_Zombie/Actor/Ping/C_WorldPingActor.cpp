// Fill out your copyright notice in the Description page of Project Settings.


#include "C_WorldPingActor.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Components/SplineMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Niagara/Internal/NiagaraSystemEmitterState.h"
#include "UI/Ping/C_PingWidget.h"
#include "Utility/C_Util.h"


AC_WorldPingActor::AC_WorldPingActor()
{
	PrimaryActorTick.bCanEverTick = true;

	m_PingWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("PingWidgetComponent");
	m_SplineMeshComponent = CreateDefaultSubobject<USplineMeshComponent>("SplineMeshComponent");
	m_RootSceneComp       = CreateDefaultSubobject<USceneComponent>("RootSceneComponent");
	
	SetRootComponent(m_RootSceneComp);
}

void AC_WorldPingActor::BeginPlay()
{
	Super::BeginPlay();

	// PingWidget의 경우, NativeConstruct에서 안보이게끔 처리함
	m_PingWidget = Cast<UC_PingWidget>(m_PingWidgetComponent->GetWidget());
	
	if (!m_PingWidget)
	{
		UC_Util::Print("From AC_WorldPingActor::BeginPlay : PingWidget casting failed", FColor::Red, 10.f);
		return;
	}
	
	m_SplineMeshComponent->SetHiddenInGame(true);
	
	// Effect의 경우, 직접적인 Spawn 처리가 들어가야 실질적으로 Spawn 처리됨(처음에는 어차피 보이지 않음)
}

void AC_WorldPingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_WorldPingActor::SpawnPingActorToWorld
(
	const FVector&	_SpawnLocation,
	EGamePingType	_PingType,
	EPingShapeType	_PingShapeType
)
{
	HidePing(); // 이전 핑 지우기용 처리

	// 핑 사운드 재생
	if (m_PingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), m_PingSound, _SpawnLocation);
	}

	// IconPing인 경우, 다리 없이 Icon 모양만 보이기
	if (_PingShapeType == EPingShapeType::IconPing)
	{
		m_PingWidgetComponent->SetWorldLocation(_SpawnLocation);
		m_PingWidget->ShowPingWidget(_SpawnLocation, _PingType);
		return;
	}
	
	// FullPing인 경우
	
	// Adjust spline position & show
	m_SplineMeshComponent->SetHiddenInGame(false);
	m_SplineMeshComponent->SetStartPosition(_SpawnLocation);
	const FVector SplineEndPos = _SpawnLocation + FVector::UnitZ() * 175.f;
	m_SplineMeshComponent->SetEndPosition(SplineEndPos);

	// Spawn Ping Effect
	m_PingEffectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation
	(
		GetWorld(),
		m_PingEffect,
		_SpawnLocation,		// 스폰할 위치 (FVector)
		GetActorRotation(), // 스폰할 회전 (FRotator)
		FVector(1.0f),      // 스케일 (FVector)
		false,              // AutoDestroy (재생 완료 후 자동 소멸 여부)
		true				// AutoActivate (스폰 즉시 재생 여부)
	);

	// Adjusting WidgetComponent Location
	m_PingWidgetComponent->SetWorldLocation(SplineEndPos + FVector::UnitZ() * 25.f);
	m_PingWidget->ShowPingWidget((_SpawnLocation + SplineEndPos) * 0.5f, _PingType);
}

void AC_WorldPingActor::HidePing()
{
	// Hide PingEffect
	if (m_PingEffectComp) m_PingEffectComp->DeactivateImmediate();
	m_PingEffectComp = nullptr;
	
	// Hide Spline
	m_SplineMeshComponent->SetHiddenInGame(true);
	
	// Hide Ping Widget
	m_PingWidget->HidePingWidget();
}

void AC_WorldPingActor::SetPingColor(const FColor& _Color)
{
	if (!m_PingWidget)
	{
		UC_Util::Print("From AC_WorldPingActor::SetPingColor : PingWidget not created", FColor::Red, 10.f);
		return;
	}
	
	m_PingWidget->SetPingMarkerColor(_Color);
}

