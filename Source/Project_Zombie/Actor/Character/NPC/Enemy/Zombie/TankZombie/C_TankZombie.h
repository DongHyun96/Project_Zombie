// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "C_TankZombie.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_TankZombie : public AC_Zombie
{
	GENERATED_BODY()

protected:
	// 충돌판정 박스
	// 돌진중에만 활성화
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChargeComponent")
	class UBoxComponent*			m_ChargeCollision;

	// 현재 실행중인 스킬 데이터
	UPROPERTY()
	class UC_EnemySkillData*		m_Skill;

	// 현재 돌진중인지 확인
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charge")
	bool							m_bCharging;

	// 돌진 시작 시 저장한 플레이어 방향
	// 플레이어가 이동해도 유도탄처럼 따라가지않음
	FVector							m_ChargeDirection;

	// 돌진 거리 계산 시 필요한 시작 위치
	FVector							m_ChargeStartLocation;

	// 계산된 돌진 속도
	// 기본 이동속도 * SkillData의 MoveSpeedScale
	float							m_ChargeSpeed;

	// 이미 충돌한 대상
	// 중복으로 충돌하는 것을 방지
	TSet<TWeakObjectPtr<AActor>>	m_ChargeHitTarget;

	// 몽타주 재생 후 실제 돌진에 사용할 대상
	UPROPERTY()
	TObjectPtr<AActor>				m_ChargeTarget;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	void UpdateCharge();

	/// <summary>
	/// 플레이어 충돌처리(데미지 + 넉백)
	/// </summary>
	void HandlePlayerHit(class AC_BasicPlayer* _Player);

	/// <summary>
	/// 좀비 충돌처리(넉백만)
	/// </summary>
	void HandleEnemyHit(class AC_BasicEnemy* _Enemy);

	UFUNCTION()
	void OnChargeBeginOverlap(
							UPrimitiveComponent* OverlappedComponent,
							AActor* OtherActor,
							UPrimitiveComponent* OtherComp,
							int32 OtherBodyIndex,
							bool bFromSweep,
							const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable, Category = "Charge")
	void StartCharge(AActor* _Target, class UC_EnemySkillData* _SkillData);

	UFUNCTION(BlueprintCallable, Category = "Charge")
	void StopCharge();

public:
	/// <summary>
	/// 돌진 시작 전 호출되는함수
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Charge")
	bool PrepareCharge(AActor* _Target, class UC_EnemySkillData* _Data);

	/// <summary>
	/// 몽타주 notify에서 호출
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Charge")
	void BeginPreparedCharge();

	UFUNCTION(BlueprintPure, Category = "Charge")
	bool IsCharging() const { return m_bCharging; }

	UFUNCTION(BlueprintCallable, Category = "Charge")
	void CancelPrepareCharge();

	void FinishChargeSkill();

public:
	AC_TankZombie();
};
