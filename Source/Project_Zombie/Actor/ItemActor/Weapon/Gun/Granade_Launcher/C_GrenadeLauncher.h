// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "C_GrenadeLauncher.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_GrenadeLauncher : public AC_GunBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Grenade", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AActor> m_GrenadeClass;

	bool m_bCanFire = true;
	FTimerHandle m_ShotCooldownTimer;
	FTimerHandle m_ReloadTimerHandle;

private:
	void StartReload();
	void CompleteReload();
	void SpawnGrenadeProjectile(const FVector& TargetPoint);
	void ResetFireCooldown();
	FVector GetCameraTargetPoint() const;

protected:
	virtual void PullTrigger() override;
	
	virtual void SpawnShellEject() override;
	virtual void PlayFireEffects_Client() override;
	virtual void AIFire(const FVector& TargetLocation) override;

	// 클라이언트 사격 시 서버에 발사체 스폰 요청
	virtual void Server_ExecuteFire_Implementation(FVector_NetQuantize ImpactPoint, AActor* HitActor) override;

	virtual void Server_StartReload_Implementation() override;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_EjectAllSpentShells(int32 SpentShellCount);

protected:
	virtual bool OnStartFire(AC_BasicPlayer* _WeaponUser) override;
	virtual bool OnFireOnGoing(AC_BasicPlayer* _WeaponUser) override;
	virtual bool OnFireEnd(AC_BasicPlayer* _WeaponUser) override;
	virtual bool Reload(AC_BasicPlayer* _WeaponUser) override;
	
public:
	AC_GrenadeLauncher();
	
	
};