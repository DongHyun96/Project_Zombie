// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "C_GranadeLauncher.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_GranadeLauncher : public AC_GunBase
{
	GENERATED_BODY()

private:
	// 스폰할 유탄 블루프린트/클래스
	UPROPERTY(EditDefaultsOnly, Category = "Grenade", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AActor> m_GrenadeClass;

	// 누적된 탄피(사용한 유탄 탄피) 개수
	int32 m_SpentShellCount = 0;

	bool m_bCanFire = true;           // 단발 발사 쿨타임 관리용
	FTimerHandle m_ShotCooldownTimer; // 쿨타임 타이머 핸들
	FTimerHandle m_ReloadTimerHandle; // 재장전 타이머 핸들

public:
	virtual bool OnStartFire(AC_BasicPlayer* _WeaponUser) override;
	virtual bool OnFireOnGoing(AC_BasicPlayer* _WeaponUser) override;
	virtual bool OnFireEnd(AC_BasicPlayer* _WeaponUser) override;
	virtual bool Reload(AC_BasicPlayer* _WeaponUser) override;

protected:
	virtual void PullTrigger() override;
	virtual void ReleaseTrigger() override;

private:
	void StartReload();
	void CompleteReload();
	void PlayFireEffects();

	void EjectAllSpentShells();

	// 유탄(Projectile) 액터 스폰 처리 함수
	void SpawnGrenadeProjectile();

	// 사격 쿨타임 해제
	void ResetFireCooldown();

public:
	AC_GranadeLauncher();
};