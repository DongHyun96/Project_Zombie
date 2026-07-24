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
	// 샷건 특화 수치 (필요 시 DataTable에서 세팅 가능)
	UPROPERTY(EditDefaultsOnly, Category = "Shotgun", meta = (AllowPrivateAccess = "true"))
	int32 m_PelletCount = 8;          // 1회 발사 시 나가는 탄알(펠릿) 수

	UPROPERTY(EditDefaultsOnly, Category = "Shotgun", meta = (AllowPrivateAccess = "true"))
	float m_SpreadAngle = 6.0f;       // 산탄 퍼짐 각도 (집탄률)

	// 1발 넣는 데 걸리는 시간 (애니메이션 길이에 맞게 조절 가능)
	UPROPERTY(EditDefaultsOnly, Category = "Shotgun|Reload", meta = (AllowPrivateAccess = "true"))
	float m_SingleShellInsertTime = 0.6f;

	bool m_bCanFire = true;           // 단발 펌프액션 쿨타임 관리용

	FTimerHandle m_ShotCooldownTimer; // 쿨타임 타이머 핸들

	FTimerHandle m_ReloadLoopTimer; // 재장전 루프용 타이머 핸들

private:
	// 샷건 산탄(펠릿) 다중 처리 함수
	void ProcessShotgunPellets(float BaseDamagePerPellet);

	// 펌프액션 / 사격 딜레이 쿨타임 해제 함수
	void ResetFireCooldown();

	// 샷건 1발씩 재장전
	void StartReload();        // 재장전 시작

	void InsertSingleShell();   // 1발 삽탄 처리

	void EndReload();          // 재장전 완료/중단

	/// <summary>
	/// R 키 입력 (재장전 처리)
	/// </summary>
	void Gun_Reload();

	/// <summary>
	/// R 키 입력 (재장전 완료 후처리)
	/// </summary>
	void CompleteReload();

	/// <summary>
	/// 방아쇠를 당겼을 때 애니메이션 및 사격 이펙트 재생
	/// </summary>
	void PlayFireEffects();

protected:
	/// <summary>
	/// 마우스 왼쪽 버튼 클릭 (사격 시작)
	/// </summary>
	virtual void PullTrigger() override;

	/// <summary>
	/// 마우스 왼쪽 버튼 클릭 뗌 (사격 중지)
	/// </summary>
	virtual void ReleaseTrigger() override;

public:
	/// <summary>
	/// 발사 시작 동작 처리 (기본 키 : LMB Started (발사 키 클릭 이벤트 발생 시))
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : StartFire 처리가 필요없거나, 모종의 이유로 실패했을 경우 return false </returns>
	virtual bool OnStartFire(AC_BasicPlayer* _WeaponUser) override;

	/// <summary>
	/// 발사 동작 지속동작 처리 (기본 키 : LMB OnGoing (계속 누르고 있는 시점) )
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : FireOnGoing 처리가 필요없거나 모종의 이유로 실패했을 경우 return false </returns>
	virtual bool OnFireOnGoing(AC_BasicPlayer* _WeaponUser) override;

	/// <summary>
	/// 발사 끝났을 때 처리 (기본 키 : LMB Completed (발사키 떼었을 때 시점) )
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : FireEnd 처리가 필요없거나 모종의 이유로 실패했을 경우 return false </returns>
	virtual bool OnFireEnd(AC_BasicPlayer* _WeaponUser) override;

	/// <summary>
	/// Reload 키 동작 처리 (기본 키 R키 기능) 
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : R키에 대한 처리가 필요없거나 실패했을 경우 return false </returns>
	virtual bool Reload(AC_BasicPlayer* _WeaponUser) override;

public:
	AC_ShotGun();
};