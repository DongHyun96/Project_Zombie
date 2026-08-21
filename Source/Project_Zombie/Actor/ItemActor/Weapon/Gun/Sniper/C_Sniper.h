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

public:
	
	virtual bool OnStartFire(AC_BasicPlayer* _WeaponUser) override;
	
private:
	void ResetFireCooldown();

protected:
	

	virtual void AIFire(const FVector& TargetLocation) override;

public:
	AC_Sniper();
};