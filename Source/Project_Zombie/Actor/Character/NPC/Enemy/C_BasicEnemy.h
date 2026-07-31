// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/NPC/C_BasicNPC.h"
#include "C_BasicEnemy.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_BasicEnemy : public AC_BasicNPC
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "SkillComponent"))
	class UC_EnemySkillComponent*			m_SkillCom;

private:
	
	UPROPERTY()
	class AC_ZombieController* m_ZombieController{};
	
protected:
	
	// 힐을 받았을 때 활성화시킬 NiagaraEffectComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Niagara")
	class UNiagaraComponent* m_HealedEffectNGComponent{};

private: /* 자기 자신을 힐러에게 힐등록할 수 있는 갯수 제한 관련 */
	
	static const int8 s_MaxHealRequestRegisterCount; // 최대 힐 Register 등록 가능 횟수 (2회(또는 2마리)로 제한)
	int8 m_HealRequestRegisterCount{}; // 힐 요청 Request 등록 count
	
public:
	
	AC_BasicEnemy();

protected:
	
	virtual void BeginPlay() override;
	
public:

	virtual float TakeDamage
	(
		float				_DamageAmount,
		FDamageEvent const& _DamageEvent,
		AController*		_EventInstigator,
		AActor*				_DamageCauser
	) override;

public:
	
	UC_EnemySkillComponent* GetSkillComponent() const { return m_SkillCom; }

	AC_ZombieController* GetZombieController() const { return m_ZombieController; }
	
private:
	
	void OnHPIncreased(AC_BasicCharacter* _HPIncreasedCharacter);

	/// <summary>
	/// 힐 받았을 때 Effect 동기화 시 호출처리
	/// </summary>
	/// <param name="_Activate"></param>
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ToggleHealedEffect(bool _Activate);

protected:
	
	/// <summary>
	/// 사망 시 호출받는 Delegate -> HealedEffect 활성화 중이었다면 해당 Effect 끄기 (및 기타 처리 여기서 할 것)
	/// </summary>
	/// <param name="_DeadCharacter"> : 죽은 캐릭터 (자기자신) </param>
	virtual void OnDead(AC_BasicCharacter* _DeadCharacter);
	
public:
	
	void DecreaseHealRequestRegisterCount();

};
