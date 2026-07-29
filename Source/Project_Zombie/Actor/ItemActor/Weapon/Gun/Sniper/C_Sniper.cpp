// Fill out your copyright notice in the Description page of Project Settings.

#include "C_Sniper.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "TimerManager.h"

AC_Sniper::AC_Sniper()
{
	m_FireMode = EFireMode::Single;
	m_SpreadAngle = 0.0f; // 스나이퍼는 탄 퍼짐 0
}

void AC_Sniper::PullTrigger()
{
	if (m_bIsFiring || m_bIsReloading || !m_bCanFire || m_CurrentAmmo <= 0) return;

	m_bIsFiring = true;
	m_bCanFire = false;

	Client_ExecuteFire();

	if (GetWorld() && m_FireRate > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(m_ShotCooldownTimer, this, &AC_Sniper::ResetFireCooldown, m_FireRate, false);
	}
	else
	{
		ResetFireCooldown();
	}
}

void AC_Sniper::ResetFireCooldown()
{
	m_bCanFire = true;
	m_bIsFiring = false;
}