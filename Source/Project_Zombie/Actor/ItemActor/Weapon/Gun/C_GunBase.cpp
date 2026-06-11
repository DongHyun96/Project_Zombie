// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GunBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "TimerManager.h"

AC_GunBase::AC_GunBase()
{
	PrimaryActorTick.bCanEverTick = true;

	m_WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = m_WeaponMesh;
}

void AC_GunBase::BeginPlay()
{
	Super::BeginPlay();
	
	m_CurrentAmmo = m_MaxAmmo;
}

void AC_GunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AC_GunBase::PullTrigger()
{
	if (m_bIsFiring) return; // 이미 쏘고 있다면 중복 실행 방지
	m_bIsFiring = true;

	// 누르자마자 딜레이 없이 즉시 한 발 발사
	PlayFireEffects();

	// m_FireRate(연사 속도) 간격으로 PlayFireEffects 함수를 무한 반복 호출
	// 마지막 인자인 true가 반복
	GetWorldTimerManager().SetTimer(m_FireTimerHandle, this, &AC_GunBase::PlayFireEffects, m_FireRate, true);
}

void AC_GunBase::ReleaseTrigger()
{
	if (!m_bIsFiring) return;
	m_bIsFiring = false;

	// 작동 중이던 연사 타이머 중지
	GetWorldTimerManager().ClearTimer(m_FireTimerHandle);
}

void AC_GunBase::Reload()
{
}

void AC_GunBase::PlayFireEffects()
{
}

