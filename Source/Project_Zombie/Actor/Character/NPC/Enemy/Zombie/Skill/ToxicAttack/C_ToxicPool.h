// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_ToxicPool.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_ToxicPool : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* m_Sphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UNiagaraComponent* m_NiagaraCom; // 나이아가라 재생 컴포넌트

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	class UNiagaraSystem* m_PoolEffect; // 투사체 시각효과 이펙트

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	class AC_BasicEnemy* m_SkillUser; // 투사체 생성자

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	class UC_EnemySkillData* m_Skill; // 투사체를 생성시킨 스킬

public:
	void InitPool(AC_BasicEnemy* _SkillUser, UC_EnemySkillData* _Skill);

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
	AC_ToxicPool();
};
