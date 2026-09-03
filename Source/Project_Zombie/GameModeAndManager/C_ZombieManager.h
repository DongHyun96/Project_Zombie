// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "UObject/Object.h"
#include "C_ZombieManager.generated.h"

class AC_SpawnArea;

/**
 * Zombie 스폰 및 Zombie 객체 Holder (서버 쪽에서만 유효, 애초에 스폰과 풀로 돌아가는 처리는 서버 쪽 기반에서 판단을 함)
 * Zombie가 날리는 Projectile도 마찬가지
 */
UCLASS(Blueprintable)
class PROJECT_ZOMBIE_API UC_ZombieManager : public UObject
{
	GENERATED_BODY()

public:
	
	UC_ZombieManager();

	void OnWorldBeginPlay();

public:

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

	const TSet<AC_Zombie*>& GetActiveNurseZombies() const { return m_ActiveZombies[EZombieType::NurseZombie]; }

private:
	
	/// <summary>
	/// 게임 시작 시 설정된 좀비 타입, 수량만큼 미리 생성하여
	/// 비활성 초기 풀 구성
	/// 서버에서만 실행
	/// </summary>
	void InitializeZombiePool();

	/// <summary>
	/// 지정한 타입의 좀비를 풀에서 꺼내
	/// 전달받은 위치에서 스폰
	/// </summary>
	/// <param name="_ZombieType"> 활성화된 좀비 </param>
	/// <param name="_SpawnTransform"> 스폰 위치 </param>
	/// <returns> 실패하면 nullptr </returns>
	AC_Zombie* SpawnZombieFromPool(EZombieType _ZombieType, const FTransform& _SpawnTransform);
	
public:
	
	/// <summary>
	/// 죽음 처리가 끝난 Zombie를 대기 Pool로 반환
	/// </summary>
	/// <param name="_Zombie"> : Pool로 반환할 Zombie </param>
	/// <returns> : 반환에 실패하면 false </returns> 
	bool ReturnZombieToPool(class AC_Zombie* _Zombie);

	/// <summary>
	/// 현재 거점 웨이브 좀비 스폰 루프 시작
	/// </summary>
	/// <param name="_SpawnArea"> 이번 웨이브에서 사용할 SpawnArea 목록 </param>
	/// <param name="_Settings"> 스폰간격, 최대 활성 수 등,, 웨이브 설정 </param>
	/// <returns></returns>
	bool StartSpawnLoop(const TArray<AC_SpawnArea*>& _SpawnArea, const FZombieWaveSetting& _Settings);

	/// <summary>
	/// 현재 진행중인 웨이브 좀비스폰 루프 중지
	/// 이미 나와있는 좀비는 그대로 두고 신규 스폰만 중지
	/// </summary>
	void StopSpawnLoop();

	/// <summary>
	/// 수동적으로 직접 ActiveZombies 컨테이너에 Active한 Zombie 집어넣기 (주의 : Level에 직접 배치시킨 좀비의 경우에만 한해 이 함수 사용중)
	/// </summary>
	/// <returns> : Valid하지 않은 ZombieType이거나 Zombie 자체가 Valid하지 않다면 집어넣지 않고 return false </returns>
	bool AddZombieToActivePoolManually(EZombieType _ZombieType, AC_Zombie* _Zombie);	

private:
	
	/// <summary>
	/// 현재 웨이브 스폰구역 중
	/// 해당 좀비타입을 허용하는 영역 중 하나를 랜덤으로 선택
	/// </summary>
	/// <returns> 사용 가능한 스폰 구역이 없으면 nullptr </returns>
	AC_SpawnArea* SelectSpawnAreaForZombieType(EZombieType _ZombieType) const;

	/// <summary>
	/// 지정된 SpawnArea에서 안전한 위치를 찾아
	/// 지정된 타입의 좀비를 Pool에서 활성화
	/// </summary>
	/// <returns></returns>
	bool TrySpawnZombieFromArea(EZombieType _ZombieType, AC_SpawnArea* _SpawnArea);

private:
	
	/// <summary>
	/// SpawnInterval마다 호출
	/// 실제 좀비 스폰 시도
	/// </summary>
	void HandleSpawnLoopTick();

	/// <summary>
	/// 여러가지 상황을 다 검사해서
	/// 해당 좀비 타입이 지금 스폰 가능한지 반환
	/// </summary>
	bool CanSpawnZombieType(const FZombieTypeSpawnSetting& _Setting) const;

	/// <summary>
	/// 현재 스폰 가능한 좀비 타입 중
	/// 스폰 가중치를 기준으로 하나 선택
	/// </summary>
	/// <returns>
	/// 성공시 Setting 주소,
	/// 후보가 없으면 nullptr
	/// </returns>
	const FZombieTypeSpawnSetting* SelectZombieTypeToSpawn() const;

	/// <summary>
	/// 타입이 실제 스폰에 성공 시 다음 스폰 시간을 기록
	/// </summary>
	void StartZombieSpawnCooldown(const FZombieTypeSpawnSetting& _Setting);

	/// <summary>
	/// 현재 필드에 활성화 된 특정 타입의 좀비 수 반환
	/// </summary>
	int32 GetActiveZombieCount(EZombieType _ZombieType) const;

	/// <summary>
	/// 해당 좀비의 연출 bool값 반환
	/// </summary>
	/// <param name="_Type"></param>
	/// <returns></returns>
	bool ShouldPlaySpecialZombieIntro(EZombieType _Type);

	void UpdateSpecialZombieCamera();

	void EndSpecialZombieIntro();


protected: /* 좀비 스폰 기반 클래스 정보 및 PoolCount 정보 -> TODO : ZombieManager 블루프린트에 해당값 초기화시킬 것 */

	// 스폰시킬 좀비 클래스들
	UPROPERTY(EditDefaultsOnly, Category = "Zombie")
	TMap<EZombieType, TSubclassOf<class AC_Zombie>> m_ZombieClasses{};

	// 노말좀비 외형 2개 랜덤 BP
	TArray<TSubclassOf<AC_Zombie>> m_NormalZombieClasses;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie")
	TMap<EZombieType, uint32> m_PoolCounts{};

private:

	// 스폰 대기중인 Active하지 않은 Zombie Pool -> Active한 좀비 Type들을 파악해야 하는 경우, 
	// Active한 좀비들은 따로 Container 만들어두기
	TMap<EZombieType, TArray<AC_Zombie*>> m_ZombiePool{}; 

	// 현재 필드에 활성화 된 좀비 목록
	TMap<EZombieType, TSet<AC_Zombie*>> m_ActiveZombies;


protected: /* Wave Spawn 관련 */

	// 현재 Spawnloop가 진행중인지
	bool m_bSpawnLoopActive = false;

	// 현재 진행중인 스폰 설정
	FZombieWaveSetting m_CurrentWaveSetting;

	// 현재 웨이브에서 사용할 SpawnArea 목록
	UPROPERTY()
	TArray<TObjectPtr<class AC_SpawnArea>>
		m_CurrentSpawnAreas;

	// 정해진 간격마다 Spawn을 실행할 타이머
	FTimerHandle m_SpawnLoopTimer;

	// 각 ZombieType이 다시 스폰 가능해지는 시간
	// 쿨타임 이후 스폰 가능해지는 시간
	TMap<EZombieType, float> m_NextZombieSpawnTime;

	
protected: /* Healer 좀비 관련 */

	/* Healer 좀비가 사용할 Healing Projectile pooling 관련 */
	
	UPROPERTY(EditDefaultsOnly, Category = "Zombie", DisplayName = "HealingProjectileClass")
	TSubclassOf<AC_HealingProjectile> m_HealingProjectileClass{};

	UPROPERTY(VisibleAnywhere)
	TArray<AC_HealingProjectile*> m_HealingProjectilePool{}; // 스폰 대기중인 HealingProjectile pool 
	

protected: /* 특수좀비 연출 관련 */
	bool m_bToxicIntroPlayed = false;
	bool m_bTankIntroPlayed = false;
	bool m_bNurseIntroPlayed = false;
	bool m_bCopIntroPlayed = false;

	FTimerHandle m_SpecialZombieCameraTimer;
	FTimerHandle m_SpecialZombieCameraEndTimer;

	TObjectPtr<AC_Zombie> m_SpecialZombieIntroZombie = nullptr;

};
