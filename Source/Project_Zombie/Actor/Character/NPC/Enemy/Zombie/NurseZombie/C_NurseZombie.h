// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "C_NurseZombie.generated.h"

/// <summary>
/// Nurse 좀비의 MainAction 갈래들 
/// </summary>
UENUM(BlueprintType)
enum class ENurseZombieActionState : uint8
{
	Idle,		// Default 상태
	Attack,		// 일반 Zombie의 Attacking 처리를 실행해야하는 상황
	Healing,	// Nurse 좀비만의 기능인 Healing Skill을 발동해야하는지
	END		UMETA(Hidden)
};

UCLASS()
class PROJECT_ZOMBIE_API AC_NurseZombie : public AC_Zombie
{
	GENERATED_BODY()

public:
	
	AC_NurseZombie();

protected:
	
	virtual void BeginPlay() override;

public:
	
	const TArray<AC_BasicEnemy*>& GetHealTargets() const { return m_HealTargets; }

	/// <summary>
	/// HealTarget으로 등록 시도 
	/// </summary>
	/// <returns> : 등록 불가능한 상태라면 return false </returns>
	bool TryRegisterAsHealTarget(AC_BasicEnemy* _NewHealTarget);

	/// <summary>
	/// Heal 타겟 제거
	/// </summary>
	/// <param name="_HealTarget"></param>
	void RemoveHealTarget(AC_BasicEnemy* _HealTarget);

private:
	
	/// <summary>
	/// Heal Target의 죽음 or 이미 FullHP를 채웠을 경우, HealTargets 컨테이너에서 해당하는 캐릭터 제거 
	/// </summary>
	UFUNCTION()
	void OnHealTargetDeadOrReachedFullHP(AC_BasicCharacter* _StatComOwner);

protected:
	
	UPROPERTY(VisibleAnywhere)
	ENurseZombieActionState m_ActionState{};
	
protected:

	UPROPERTY(VisibleAnywhere)
	TArray<AC_BasicEnemy*> m_HealTargets{};

private:

	// 3 마리까지 Heal Target을 잡을 수 있도록 한다
	static const uint8 s_HealTargetCountLimit;
	
};
