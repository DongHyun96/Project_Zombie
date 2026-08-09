// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_ToxicPool.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS()
class PROJECT_ZOMBIE_API AC_ToxicPool : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent*		m_Sphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UNiagaraComponent*	m_NiagaraCom; // 나이아가라 재생 컴포넌트

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	class UNiagaraSystem*		m_PoolEffect; // 장판 시각효과 이펙트

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	class AC_BasicEnemy*		m_SkillUser; // 장판 생성자

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	class UC_EnemySkillData*	m_Skill; // 장판을 생성시킨 스킬

	// 현재 장판안에 있는 대상
	// 중복으로 등록되면 안되기때문에 TArray가 아니라 TSet 사용
	UPROPERTY()
	TSet<TObjectPtr<AActor>>	m_OverlapTargets; 

	// 데미지 반복 타이머
	FTimerHandle				m_DamageTimer;

	// 장판 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pool")
	float m_PoolLifeTime = 10.f; // 장판 수명

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pool")
	float m_DamageInterval = 1.f; // 틱데미지 간격

protected:

	UPROPERTY(VisibleAnywhere, Category = "Sound")
	TObjectPtr<UAudioComponent> m_AudioCom;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> m_LoopSound;
	
public:
	/// <summary>
	/// 장판 초기화 함수
	/// </summary>
	void InitPool(AC_BasicEnemy* _SkillUser, UC_EnemySkillData* _Skill);

protected:
	virtual void BeginPlay() override;

	/// <summary>
	/// 데미지를 받을 대상을 등록
	/// </summary>
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/// <summary>
	/// 일정 간격으로 실행되는 데미지 함수
	/// </summary>
	void ApplyTickDamage();

public:
	AC_ToxicPool();
};
