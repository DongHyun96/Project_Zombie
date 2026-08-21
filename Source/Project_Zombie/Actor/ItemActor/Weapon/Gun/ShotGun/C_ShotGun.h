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

public:
	AC_ShotGun();

	virtual bool OnStartFire(AC_BasicPlayer* _WeaponUser) override;

	virtual void OnSheathStart() override;

private:
	
	virtual bool ExecuteFire() override;
	virtual void AIFire(const FVector& TargetLocation) override;

	UFUNCTION(Server, Reliable)
	void Server_ShotgunFireEffects(const TArray<FVector_NetQuantize>& ImpactPoints);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayShotgunFireEffects(const TArray<FVector_NetQuantize>& ImpactPoints);

private:
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopReloadAnimation();

public:
	
	/// <summary>
	/// SingleReload AN 도달 시, 호출 
	/// </summary>
	virtual void AN_OnSingleReloadEnd() override;

	/// <summary>
	/// 빈 호출로 통상적으로 GunReloadEnd 처리 자체를 기능하지 못하도록 막음 (ShotGun의 경우, SingleReloadEnd로 재장전 처리를 모두 수행한다)
	/// </summary>
	virtual void AN_OnGunReloadEnd() override {}
	
private:
	
	void ResetFireCooldown();

private:
	int32 m_PelletCount;
	
	FTimerHandle m_ShotCooldownTimer;
	bool m_bCanFire = false;
};