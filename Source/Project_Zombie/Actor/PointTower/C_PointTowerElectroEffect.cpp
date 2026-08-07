// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PointTowerElectroEffect.h"

#include "C_PointTower.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Components/SplineComponent.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Net/UnrealNetwork.h"


AC_PointTowerElectroEffect::AC_PointTowerElectroEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SetReplicates(true);
	bNetUseOwnerRelevancy = true;
}

void AC_PointTowerElectroEffect::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorHiddenInGame(true);

	if (HasAuthority())
		m_OwnerPointTower = Cast<AC_PointTower>(GetOwner()); // Replication 이용
}

void AC_PointTowerElectroEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!m_TargetPlayer || !m_OwnerPointTower) return;
	
	// OwnerPointTower와 TargetPlayer 모두 Valid한 상황 -> 실시간으로 끝점 연결처리
	SetActorLocation(m_TargetPlayer->GetActorLocation() + FVector::UnitZ() * 25.f);
	
	// 끝점을 OwnerPointTower의 Generator로 위치시키기
	// 1. 끝점 인덱스 구하기 (0-based 인덱스)
	const int32 LastPointIndex = m_Spline->GetNumberOfSplinePoints() - 1;

	if (LastPointIndex >= 0)
	{
		// 2. Generator의 월드 위치 가져오기
		const FVector TargetWorldLocation = m_OwnerPointTower->GetGenerator()->GetComponentLocation();

		// 3. Spline의 맨 끝점을 TargetWorldLocation 위치로 이동
		m_Spline->SetLocationAtSplinePoint
		(
			LastPointIndex, 
			TargetWorldLocation, 
			ESplineCoordinateSpace::World, 
			true // True로 설정 시 위치 변경 후 스플라인 탄젠트/곡선을 즉시 갱신 (UpdateSpline)
		);
	}
}

void AC_PointTowerElectroEffect::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AC_PointTowerElectroEffect, m_TargetPlayer);
	DOREPLIFETIME(AC_PointTowerElectroEffect, m_OwnerPointTower);
}

void AC_PointTowerElectroEffect::SetTargetPlayer(AC_BasicPlayer* _TargetPlayer)
{
	if (!HasAuthority()) return;
	m_TargetPlayer = _TargetPlayer;
	OnRep_TargetPlayer();
}

void AC_PointTowerElectroEffect::OnRep_TargetPlayer()
{
	if (!m_TargetPlayer)
	{
		SetActorHiddenInGame(true);
		return;
	}

	SetActorHiddenInGame(false);
}
