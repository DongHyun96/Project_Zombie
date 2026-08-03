// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PointTower.h"

#include "GameModeAndManager/C_GameMode_GameLv.h"
#include "GameModeAndManager/PointTowerManager/C_PointTowerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"

const float AC_PointTower::m_MaxConquerAmount = 100.f;

AC_PointTower::AC_PointTower()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AC_PointTower::BeginPlay()
{
	Super::BeginPlay();

	// 서버 환경에서의 PointTower만 PointTowerManager(서버 쪽에만 존재) 에 등록 처리를 할 것임
	if (HasAuthority())
		GAME_LV_GAME_MODE(GetWorld())->GetPointTowerManager()->RegisterPointTower(this);		
}

void AC_PointTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AC_PointTower::Activate(float _ConquerAmount)
{
	if (!HasAuthority())
	{
		UC_Util::Print("[AC_PointTower::Activate] : Only call Activate function in server!", FColor::Red, 10.f);
		return false;
	}

	m_CurConquerAmount = _ConquerAmount;
	Multicast_Activate(_ConquerAmount);
	
	return true;
}

void AC_PointTower::Multicast_Activate_Implementation(float _ConquerAmount)
{
	// TODO : 거점 활성화 이펙트 처리
	
}

