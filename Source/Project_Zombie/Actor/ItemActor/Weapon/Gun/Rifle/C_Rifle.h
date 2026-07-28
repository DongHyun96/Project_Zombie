// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "C_Rifle.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API AC_Rifle : public AC_GunBase
{
	GENERATED_BODY()

private:
	FTimerHandle m_AutoFireTimer;
	FTimerHandle m_BurstCooldownTimer;

	const int32 m_MaxBurstCount = 3;
	const float m_BurstCooldown = 0.2f;

	int32 m_BurstCount = 0;              
	bool m_bInBurstCooldown = false;

private:
	void HandleAutomaticFire();
	void HandleBurstFire();
	void ResetBurstCooldown();
	void ProcessSingleRifleShot(float DamageVal);

protected:
	virtual void PullTrigger() override;
	virtual void ReleaseTrigger() override;

	virtual void Server_ExecuteFire() override;

	// 사격 모드 전환 
	virtual void SwitchFireMode() override;

public:
	AC_Rifle();
};