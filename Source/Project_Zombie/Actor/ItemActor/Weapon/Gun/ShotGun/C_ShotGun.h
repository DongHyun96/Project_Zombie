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
	virtual bool Reload(AC_BasicPlayer* _WeaponUser) override;

	virtual void EndReload();
	virtual void OnSheathStart() override;

protected:
	virtual void PullTrigger() override;
	virtual void Client_ExecuteFire() override;
	virtual void AIFire(const FVector& TargetLocation) override;

	UFUNCTION(Server, Reliable)
	void Server_ShotgunFireEffects(const TArray<FVector_NetQuantize>& ImpactPoints);

	virtual void Server_StartReload_Implementation() override;

	// 클라이언트 UI 및 탄약 동기화 RPC
	UFUNCTION(Client, Reliable)
	void Client_OnSingleShellInserted(int32 NewAmmo);

	// 클라이언트 재장전 종료 동기화 RPC
	UFUNCTION(Client, Reliable)
	void Client_EndReload();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayShotgunFireEffects(const TArray<FVector_NetQuantize>& ImpactPoints);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopReloadAnimation();

private:
	void InsertSingleShell();
	void ResetFireCooldown();

private:
	int32 m_PelletCount;
	float m_SingleShellInsertTime;
	FTimerHandle m_ReloadLoopTimer;
	FTimerHandle m_ShotCooldownTimer;
	bool m_bCanFire = false;
};