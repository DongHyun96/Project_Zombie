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
	bool m_bCanFire = true;           // 볼트액션 딜레이용 쿨타임 플래그
	FTimerHandle m_ShotCooldownTimer; // 쿨타임 타이머 핸들
	FTimerHandle m_ReloadTimerHandle; // 재장전 타이머 핸들

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

	// 스나이퍼 전용 고정밀 라인트레이스 데미지 처리
	void ProcessSniperShot(float DamageVal);

	// 사격 쿨타임(볼트액션/반자동 딜레이) 해제
	void ResetFireCooldown();

public:
	AC_Sniper();
};