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

private:
	void HandleAutomaticFire();
	void ProcessSingleRifleShot(float DamageVal);

protected:
	virtual void PullTrigger() override;
	virtual void ReleaseTrigger() override;

	virtual void Server_ExecuteFire() override;

public:
	AC_Rifle();
};