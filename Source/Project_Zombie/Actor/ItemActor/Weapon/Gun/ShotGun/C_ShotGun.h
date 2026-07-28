// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "C_ShotGun.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API AC_ShotGun : public AC_GunBase
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category = "Shotgun")
	int32 m_PelletCount = 8;

	UPROPERTY(EditDefaultsOnly, Category = "Shotgun|Reload")
	float m_SingleShellInsertTime = 0.6f;

	bool m_bCanFire = true;
	FTimerHandle m_ShotCooldownTimer;
	FTimerHandle m_ReloadLoopTimer;

private:
	void ProcessShotgunPellets(float BaseDamagePerPellet);
	void ResetFireCooldown();
	void InsertSingleShell();
	void EndReload();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayShotgunFireEffects(const TArray<FVector_NetQuantize>& ImpactPoints);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopReloadAnimation();

protected:
	virtual void PullTrigger() override;
	virtual void Server_ExecuteFire() override;
	virtual void Server_ExecuteReload() override;

public:
	virtual bool OnStartFire(AC_BasicPlayer* _WeaponUser) override;

public:
	AC_ShotGun();
};