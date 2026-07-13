// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_FireDamageArea.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_FireDamageArea : public AActor
{
	GENERATED_BODY()
	
public:	
	AC_FireDamageArea();

protected:
	virtual void BeginPlay() override;

	// 장판이 끝날 때 호출되는 함수
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/// <summary>
	/// 화염 데미지 영역에 들어온 액터에게 데미지를 적용
	/// </summary>
	void ApplyPointDamage();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Effect")
	TObjectPtr<UParticleSystemComponent> m_FireEffectComponent;

	// 데미지 영역 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Damage")
	float m_DamageRadius;

	// 장판 지속 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Damage")
	float m_Duration;

	// 초당 데미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Damage")
	float m_DamagePerSecond; 

	// 데미지를 적용하는 간격 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Damage")
	float m_DamageInterval; 

	// 데미지 타입
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Damage")
	TSubclassOf<UDamageType> m_DamageType;

	// 데미지 반복 타이머 핸들
	FTimerHandle m_DamageTimerHandle;
};
