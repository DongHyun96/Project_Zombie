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

protected:
	// 탄 퍼짐 수치 0에 가까울 수록 중앙
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stat")
	float m_SpreadAngle = 1.5f;

public:
	/// <summary>
	/// 마우스 왼쪽 버튼 클릭 (사격 시작)
	/// </summary>
	virtual void PullTrigger() override;

	/// <summary>
	/// 마우스 왼쪽 버튼 클릭 뗌 (사격 중지)
	/// <summary>
	virtual void ReleaseTrigger() override;

	/// <summary>
	/// R 키 입력 (재장전 요청)
	/// <summary>
	void Gun_Reload();

	/// <summary>
	/// R 키 입력 (재장전 후처리 요청)
	/// <summary>
	void CompleteReload();

	/// <summary>
	/// 방아쇠를 당겼을 때 애니메이션 재생 함수
	/// <summary>
	void PlayFireEffects();

	/// <summary>
	/// 공통 라인트레이스 데미지 처리 (플레이어용)
	/// </summary>
	void RifleLineTraceDamage(float DamageVal, float SpreadAngleDegree);

public:
	/// <summary>
	/// 발사 시작 동작 처리 (기본 키 : LMB Started (발사 키 클릭 이벤트 발생 시))
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : StartFire 처리가 필요없거나(이건 웬만한 무기는 다 필요할 듯), 모종의 이유로 실패했을 경우 return false </returns>
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
	AC_Rifle();
};
