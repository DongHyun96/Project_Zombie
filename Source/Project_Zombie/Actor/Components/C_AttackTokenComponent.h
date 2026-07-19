// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_AttackTokenComponent.generated.h"

class AC_BasicEnemy;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_AttackTokenComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	// 동시 공격가능한 최대 적 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackToken", meta = (AllowprivateAccess = "true", ClampMin = "1", UIMin = "1"))
	int32 m_MaxAttackCount = 3;

	// 현재 토큰을 가진 적 목록
	TSet<TWeakObjectPtr<AC_BasicEnemy>>
		m_CurrentAttackers;

public:
	/// <summary>
	/// 공격토큰 획득시도 함수
	/// </summary>
	bool TryAcquireToken(AC_BasicEnemy* _Enemy);

	/// <summary>
	/// 공격토큰 반환 함수
	/// </summary>
	void ReleaseToken(AC_BasicEnemy* _Enemy);

	/// <summary>
	/// 적이 토큰을 가지고있는지 확인하는 함수
	/// </summary>
	bool HasToken(const AC_BasicEnemy* _Enemy) const;

	int32 GetCurrentAttackCount() const;

	/// <summary>
	/// 공격 가능한 자리가 남아있는지 확인하는 함수
	/// </summary>
	bool CanAcquireToken() const;

private:
	/// <summary>
	/// 죽거나 없어진 적을 목록에서 제거하는 함수
	/// </summary>
	void RemoveAttackers();

protected:
	virtual void BeginPlay() override;

public:
	UC_AttackTokenComponent();
		
};
