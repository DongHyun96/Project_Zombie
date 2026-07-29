// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "C_Sniper.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API AC_Sniper : public AC_GunBase
{
	GENERATED_BODY()

private:
	bool m_bCanFire = true;
	FTimerHandle m_ShotCooldownTimer;

private:
	void ProcessSniperShot(float DamageVal);
	void ResetFireCooldown();

protected:
	virtual void PullTrigger() override;
	virtual void Server_ExecuteFire() override;

public:
	AC_Sniper();
};