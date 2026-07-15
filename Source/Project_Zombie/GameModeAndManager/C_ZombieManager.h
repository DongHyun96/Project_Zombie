// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "C_ZombieManager.generated.h"

enum class EZombieType : uint8;
/**
 * Zombie 스폰 및 Zombie 객체 Holder
 */
UCLASS(Blueprintable)
class PROJECT_ZOMBIE_API UC_ZombieManager : public UObject
{
	GENERATED_BODY()

public:
	
	UC_ZombieManager();

	void OnWorldBeginPlay();

public: /* Spawn 관련 함수들 */

	/// <summary>
	/// Healer 좀비의 HealingProjectile 스폰 시키기
	/// </summary>
	/// <param name="_SpawnLocation"> : 스폰 위치 </param>
	/// <param name="_FireDirection"> : 발사 방향(Homing projectile을 써도 초기 Velocity 세팅은 필요) </param>
	/// <param name="_SpawnedBy"> : 이 Projectile을 스폰시킨 NurseZombie </param>
	/// <param name="_HealingTarget"> : Heal 주려는 Target Enemy </param>
	/// <param name="_TotalHealAmount"> : Projectile 충돌 시 부여할 힐량 </param>
	/// <returns> : Spawn 실패 시 return false </returns>
	bool SpawnHealingProjectile
	(
		const FVector& 			_SpawnLocation,
		const FVector& 			_FireDirection,
		class AC_NurseZombie*	_SpawnedBy,
		class AC_BasicEnemy*	_HealingTarget,
		float					_TotalHealAmount
	);

	/// <summary>
	/// Active했던 HealingProjectile에 대해, 스폰 대기 Pool로 되돌아가기
	/// </summary>
	/// <param name="_HealingProjectile"> : 대상 </param>
	/// <returns> : 제대로 Pool로 돌아가지 못했다면 return false </returns>
	bool ReturnHealingProjectileToPool(class AC_HealingProjectile* _HealingProjectile);
	
protected:

	// 스폰시킬 좀비 클래스들
	UPROPERTY(EditDefaultsOnly, Category = "Zombie")
	TMap<EZombieType, TSubclassOf<class AC_Zombie>> m_ZombieClasses{};

	UPROPERTY(EditDefaultsOnly, Category = "Zombie")
	TMap<EZombieType, uint32> m_PoolCounts{};

protected: /* Healer 좀비 Healing Projectile 관련 */

	UPROPERTY(EditDefaultsOnly, Category = "Zombie", DisplayName = "HealingProjectileClass")
	TSubclassOf<AC_HealingProjectile> m_HealingProjectileClass{};

	UPROPERTY(VisibleAnywhere)
	TArray<AC_HealingProjectile*> m_HealingProjectilePool{}; // 스폰 대기중인 HealingProjectile pool 
	
};
