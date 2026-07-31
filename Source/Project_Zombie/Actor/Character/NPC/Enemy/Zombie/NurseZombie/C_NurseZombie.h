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

	friend class UC_NurseHealing;
	
public:
	
	AC_NurseZombie();

protected:
	
	virtual void BeginPlay() override;

public:
	
	const TArray<AC_BasicEnemy*>& GetHealProjectileTargets() const { return m_HealProjectileTargets; }
	
	/// <param name="_OutOverlappingEnemies"> : Enemy 류 AActor* 객체 담아서 반환(GetOverlappingActor 함수 특성 때문에 AActor* 형식으로밖에 처리안됨) </param>
	void GetHealingAuraOverlappingEnemies(TArray<AActor*>& _OutOverlappingEnemies) const;
	
	float GetHealingAuraHPS() const { return m_HealAuraHPS; }
	
private:
	
	/// <summary>
	/// HealTarget 컨테이너에서 제거함과 동시에, Enemy 쪽 HealRequestRegisterCnt 하나 줄이기 처리 & Delegate 구독 해제 처리
	/// </summary>
	void RemoveHealProjectileTarget(AC_BasicEnemy* _Target);
	
public:
	
	/// <summary>
	/// HealTarget으로 등록 시도 
	/// </summary>
	/// <returns> : 등록 불가능한 상태라면 return false </returns>
	bool TryRegisterAsHealTarget(AC_BasicEnemy* _NewHealTarget);

public:

	// ABP 상태값으로 사용하고 있지 않음 -> 따라서 Replicate 처리할 필요 없음
	void SetActionState(ENurseZombieActionState _ActionState) { m_ActionState = _ActionState; }
	ENurseZombieActionState GetActionState() const { return m_ActionState; }

	/// <summary>
	/// 주변부 Healing Aura 활성화/비활성화
	/// </summary>
	void ToggleHealingAura(bool _Activate);
	
private:
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ToggleHealingAura(bool _Active);
	
private:
	
	/// <summary>
	/// Heal Target의 죽음 or 이미 FullHP를 채웠을 경우, HealTargets 컨테이너에서 해당하는 캐릭터 제거 
	/// </summary>
	UFUNCTION()
	void OnHealTargetDeadOrReachedFullHP(AC_BasicCharacter* _StatComOwner);

	/// <summary>
	/// Heal Skill이 끝나면 호출될 함수 
	/// </summary>
	UFUNCTION()
	void OnHealSkillEnd(AC_BasicEnemy* _Enemy);

private:
	
	virtual void OnDead(AC_BasicCharacter* _DeadCharacter) override;
	
protected:

	// 주의 : Healing ActionState라고 해서 무조건 HealSkill을 발동중이 아닐 수 있다
	// BehaviorTree에서의 MainStream을 나누기 위한 enum값일 뿐이다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ENurseZombieActionState m_ActionState{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<AC_BasicEnemy*> m_HealProjectileTargets{};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UNiagaraComponent* m_HealingAuraEffectNG{};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class USphereComponent* m_HealingAuraCollider{};	

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float m_HealAuraHPS{}; // Healing Aura의 Healing per sec 값
	
private:

	// 3 마리까지 Heal Target을 잡을 수 있도록 한다
	static const uint8 s_HealTargetCountLimit;
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UBoxComponent* m_NormalAttackCollider{};
	
};
