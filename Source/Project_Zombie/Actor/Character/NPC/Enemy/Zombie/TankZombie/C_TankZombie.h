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

	// ===========================
	// End 점프/착지 이동
	// ===========================

	// End 애니메이션 동안 실제 캡슐이 이동중인지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChargeEnd")
	bool m_bEndMoving;

	// End 애니메이션에서 이동할 방향
	FVector m_EndMoveDirection;

	// End 구간 이동속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChargeEnd")
	float m_EndMoveSpeed;

	// End 구간 위쪽 점프 힘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChargeEnd")
	float m_EndMoveUpPower;

	// 노티파이가 실행되지 않았을 때를 대비한 이동시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChargeEnd")
	float m_EndMoveMaxTime;

	// 현재 End 이동시간
	float m_EndMoveElapsedTime;


	// ===========================
	// 착지 충격파
	// ===========================

	// 충격 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChargeLanding")
	float m_LandingShockRadius;

	// 주변 대상을 위로 띄우는 힘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChargeEnd")
	float m_LandingShockUpPower;

	// 주변 대상을 바깥으로 밀어내는 힘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChargeEnd")
	float m_LandingShockOutPower;


	// ===========================
	// 돌진
	// ===========================

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

	// ==== End 관련 함수 ====

	/// <summary>
	/// End의 점프착지 애니메이션 이동시작 함수
	/// </summary>
	void StartEndMove();

	/// <summary>
	/// End 점프착지 애니메이션 이동 업데이트
	/// </summary>
	void UpdateEndMove(float DeltaTime);

	/// <summary>
	/// End 이동정지
	/// </summary>
	void StopEndMove();

	/// <summary>
	/// 착지 시 주변 대상 띄우기
	/// </summary>
	void ApplyLandingShock();



	// ==== Charge 관련 함수 ====

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
	/// End 애니메이션 착지부분 notify에서 호출
	/// 탱크 이동정지 + 주변 띄우기
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Charge")
	void LandingImpact();

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
