// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Ping/C_WorldPingActor.h"
#include "Components/ActorComponent.h"
#include "C_AIGunUsageComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_ZOMBIE_API UC_AIGunUsageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UC_AIGunUsageComponent();

protected:
	
	virtual void BeginPlay() override;

public:
	
	/// <summary>
	/// 이전 무기주인 set
	/// </summary>
	void SetPrevOwnerPlayer(class AC_BasicPlayer* _PrevOwnerPlayer) { m_PrevOwnerPlayer = _PrevOwnerPlayer; }
	
public:
	
	/// <summary>
	/// <para> Enemy AI 총기 발사 처리 </para>
	/// <para> (단순 1발 발사 처리로 처리할 것 -> Sniper같은 총기류도 Montage 이어서 재장전 모션 나오게끔 처리예정) </para>
	/// </summary>
	virtual bool AIFire();
	
	// 모든 클라이언트에게 이펙트 재생
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayAIFireEffects(FVector_NetQuantize ImpactPoint);

public: /* Attachment 처리 관련 */

	/// <summary>
	/// Enemy 손에 장착하기
	/// </summary>
	/// <param name="_ParentMesh"></param>
	/// <returns> : 실패 시 return false </returns>
	bool AttachToHand(USceneComponent* _ParentMesh);

	/// <summary>
	/// Detaching 진행과 동시에, Enemy 머리위로 SkeletalMeshComp 충돌 잠시 켜두어 Launch 처리
	/// </summary>
	/// <returns></returns>
	bool DetachFromHand();
	
private:
	
	/// <summary>
	/// AI 전용 Damage 주기 처리
	/// </summary>
	FVector AIProcessLineTraceDamage(float _DamageVal);

	/// <summary>
	/// 무기 Drop 처리 이후, GunMesh 굴러가는 게 멈췄는지 체크하고, 멈췄다면 대응되는 처리 실행
	/// </summary>
	void HandleGunMeshPhysicsStopped();
	
private:
	
	UPROPERTY()
	class AC_GunBase* m_OwnerGun{};
	
	// 현재 이 Gun을 사용중인 CopZombieUser
	UPROPERTY()
	class AC_CopZombie* m_WeaponCopZombieUser{};
	
	UPROPERTY()
	AC_BasicPlayer* m_PrevOwnerPlayer{};

private:
	
	FTimerHandle m_GunMeshStoppedCheckTimer{};
	
};
