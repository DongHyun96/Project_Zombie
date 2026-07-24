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

	// 누적된 탄피(사용한 유탄 탄피) 개수 -> 이거 이렇게 변수로 두지 말고 Max - cur로 계산하는게 좋아 보이는데.(상연)
	int32 m_SpentShellCount = 0;

	bool m_bCanFire = true;       
	FTimerHandle m_ShotCooldownTimer;
	FTimerHandle m_ReloadTimerHandle;

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