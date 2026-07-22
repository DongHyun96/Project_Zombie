// Fill out your copyright notice in the Description page of Project Settings.


#include "C_WorldPingActor.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
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

void AC_WorldPingActor::SpawnPingActorToWorld(const FHitResult& _TraceHitResult, EGamePingType _PingType)
{
	HidePing(); // 이전 핑 지우기용 처리

	// DefaultMarker에 대해서만 바닥면을 따짐 (다른 Ping 종류의 경우 무조건 FullPingActor 모습으로 스폰 처리)
	if (_PingType == EGamePingType::DefaultMarker)
	{
		// 대략 경사도 45도까지는 바닥면으로 간주
		if (_TraceHitResult.ImpactNormal.Z < 0.7f)
		{
			m_PingWidgetComponent->SetWorldLocation(_TraceHitResult.ImpactPoint);
			m_PingWidget->ShowPingWidget(_TraceHitResult.ImpactPoint);
			return;
		}
	}
	
	// 바닥면이나 DefaultMarker가 아닌 경우, 해당 위치에 전체 Actor 보이게끔 처리

	// Adjust spline position & show
	m_SplineMeshComponent->SetHiddenInGame(false);
	m_SplineMeshComponent->SetStartPosition(_TraceHitResult.ImpactPoint);
	const FVector SplineEndPos = _TraceHitResult.ImpactPoint + FVector::UnitZ() * 175.f;
	m_SplineMeshComponent->SetEndPosition(SplineEndPos);

	// Spawn Ping Effect
	m_PingEffectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation
	(
		GetWorld(),
		m_PingEffect,
		_TraceHitResult.ImpactPoint,             // 스폰할 위치 (FVector)
		GetActorRotation(),             		 // 스폰할 회전 (FRotator)
		FVector(1.0f),                  		 // 스케일 (FVector)
		false,                          		 // AutoDestroy (재생 완료 후 자동 소멸 여부)
		true									 // AutoActivate (스폰 즉시 재생 여부)
	);

	// Adjusting WidgetComponent Location
	m_PingWidgetComponent->SetWorldLocation(SplineEndPos + FVector::UnitZ() * 25.f);
	m_PingWidget->ShowPingWidget((_TraceHitResult.ImpactPoint + SplineEndPos) * 0.5f, _PingType);
}

void AC_WorldPingActor::SpawnFullPingActorToWorld(const FVector& _SpawnLocation, EGamePingType _PingType)
{
	HidePing(); // 이전 핑 지우기용 처리
	
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

