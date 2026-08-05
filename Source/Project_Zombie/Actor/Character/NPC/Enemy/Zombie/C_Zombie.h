// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../C_BasicEnemy.h"
#include "C_Zombie.generated.h"

UENUM(BlueprintType)
enum class EZombieType : uint8
{
	NormalZombie,
	ToxicZombie,
	NurseZombie,
	CopZombie,
	TankZombie
};

UCLASS()
class PROJECT_ZOMBIE_API AC_Zombie : public AC_BasicEnemy
{
	GENERATED_BODY()

protected:
	
	const EZombieType m_ZombieType{};

	/// <summary>
	/// 현재 좀비가 필드에서 활성상태인지 
	/// false = 오브젝트 풀 대기상태
	/// </summary>
	UPROPERTY(Replicated = OnRep_PoolActive)
	bool m_bPoolActive = true;

public:
	/// <summary>
	/// 현재 Zombie 종류 반환
	/// </summary>
	EZombieType GetZombieType() const { return m_ZombieType; }
	
protected: /* 공통 NormalAttack 피격판정 및 피격처리 (해당 기능이 없는 좀비의 경우 처리되지 않음) */

	// NormalAttack Collider가 여러 개일 수도 있어서(아예 근접 휘두르기 공격 자체를 안하는 좀비인 경우, 해당 AttackCollider 자체가 들어오지 않을 예정)
	// TODO : 각 Zombie에서 생성한 NormalAttackCollider들 존재한다면, 여기에 넣어둘 것
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<UShapeComponent*> m_NormalAttackColliders{};

private:
	
	// 이미 이번 NormalAttack 휘두르기에 피격판정이 들어간 Player들
	UPROPERTY()
	TSet<class AC_BasicPlayer*> m_SetNormalAttackColliderEntered{};
	
protected:
	virtual void BeginPlay() override;

public:
	
	/// <summary>
	/// 기본 공격 피격 처리 필요 시, 해당 함수 override 해줄 것 + 해당 공격 모션 Montage에 ANS_OnZombieNormalAttack 걸어둘 것
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void ANS_OnNormalAttackStart();

	/// <summary>
	/// 기본 공격 피격 처리 필요 시, 해당 함수 override 해줄 것 + 해당 공격 모션 Montage에 ANS_OnZombieNormalAttack 걸어둘 것
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void ANS_OnNormalAttackEnd();

private: /* 기본 공격처리 관련 */

	UFUNCTION()
	void OnNormalAttackColliderBeginOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor*				 OtherActor,
		UPrimitiveComponent* OtherComp,
		int32				 OtherBodyIndex,
		bool				 bFromSweep,
		const FHitResult&	 SweepResult
	);

protected:
	
	void AddNormalAttackCollider(UShapeComponent* _NormalAttackCollider) { m_NormalAttackColliders.Add(_NormalAttackCollider); }
	
	// --------- 스폰관련 ------------ //

protected:
	/// <summary>
	/// 서버 또는 클라에서 풀 활성 상태에 따라
	/// 외형과 충돌을 적용
	/// </summary>
	void ApplyPoolActiveState();

	/// <summary>
	/// 클라가 풀 활성 상태를 받았을 때 호출
	/// </summary>
	void OnRep_PoolActive();

public:
	/// <summary>
	/// 죽은 좀비를 레벨에서 제거하고 
	/// 풀 대기상태로 전환
	/// 서버에서만 호출
	/// </summary>
	void DeactivateForPool();

	bool IsPoolActive() const
	{
		return m_bPoolActive;
	}
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	virtual void Tick(float DeltaTime) override;

	/// <summary>
	/// 기본 CDO 생성 처리를 위해 기본생성자는 항상 있어야 함
	/// </summary>
	AC_Zombie();

	/// <summary>
	/// 자식 Zombie 클래스에서 자신의 ZombieType을 초기화하기 위한 생성자
	/// </summary>
	AC_Zombie(EZombieType _ZombieType);

	
};
