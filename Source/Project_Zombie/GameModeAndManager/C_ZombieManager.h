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

	const TSet<AC_NurseZombie*>& GetActiveNurseZombies() const { return m_ActiveNurseZombies; }
	

	/*// ================== 동기화 처리 후 마무리 =======================
	/// <summary>
	/// 죽음 처리가 끝난 Zombie를 대기 Pool로 반환
	/// </summary>
	/// <param name="_Zombie"> : Pool로 반환할 Zombie </param>
	/// <returns> : 반환에 실패하면 false </returns> 
	bool ReturnZombieToPool(class AC_Zombie* _Zombie);
	//==================================================================*/

public: /* For testing TODO : 이 Block 밑 지울 함수들 모두 지워버릴 것 */

	// TODO : 이 함수 테스트 때문에 넣어둠 지워버릴 것
	void AddNurseZombieToActivePool(AC_NurseZombie* _NurseZombie) { m_ActiveNurseZombies.Add(_NurseZombie); } 
	
	
protected: /* 좀비 스폰 기반 클래스 정보 및 PoolCount 정보 -> TODO : ZombieManager 블루프린트에 해당값 초기화시킬 것 */

	// 스폰시킬 좀비 클래스들
	UPROPERTY(EditDefaultsOnly, Category = "Zombie")
	TMap<EZombieType, TSubclassOf<class AC_Zombie>> m_ZombieClasses{};

	UPROPERTY(EditDefaultsOnly, Category = "Zombie")
	TMap<EZombieType, uint32> m_PoolCounts{};

private:

	// 스폰 대기중인 Active하지 않은 Zombie Pool -> Active한 좀비 Type들을 파악해야 하는 경우, 
	// Active한 좀비들은 따로 Container 만들어두기
	TMap<EZombieType, TArray<AC_Zombie*>> m_ZombiePool{}; 
	
protected: /* Healer 좀비 관련 */

	// 현재 스폰되어 레벨에 살아서 돌아다니는 Nurse 좀비들
	UPROPERTY(VisibleAnywhere)
	TSet<AC_NurseZombie*> m_ActiveNurseZombies{};
	
	/* Healer 좀비가 사용할 Healing Projectile pooling 관련 */
	
	UPROPERTY(EditDefaultsOnly, Category = "Zombie", DisplayName = "HealingProjectileClass")
	TSubclassOf<AC_HealingProjectile> m_HealingProjectileClass{};

	UPROPERTY(VisibleAnywhere)
	TArray<AC_HealingProjectile*> m_HealingProjectilePool{}; // 스폰 대기중인 HealingProjectile pool 
	
};
