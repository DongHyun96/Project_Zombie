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

protected:
	
	// 힐을 받았을 때 활성화시킬 NiagaraEffectComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Niagara")
	class UNiagaraComponent* m_HealedEffectNGComponent{};
	
public:
	void BeginPlay() override;
	AC_BasicEnemy();

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
	
private:
	
	void OnHPIncreased(AC_BasicCharacter* _HPIncreasedCharacter);
	
	/// <summary>
	/// 사망 시 호출받는 Delegate -> HealedEffect 활성화 중이었다면 해당 Effect 끄기 (및 기타 처리 여기서 할 것)
	/// </summary>
	/// <param name="_DeadCharacter"> : 죽은 캐릭터 (자기자신) </param>
	void OnDead(AC_BasicCharacter* _DeadCharacter);

};
