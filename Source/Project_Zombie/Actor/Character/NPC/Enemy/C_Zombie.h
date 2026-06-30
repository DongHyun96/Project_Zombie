// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "C_BasicEnemy.h"
#include "C_Zombie.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_Zombie : public AC_BasicEnemy
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* m_AttackMontage;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/// <summary>
	/// 공격 애니메이션 테스트용 임시 함수
	/// </summary>
	void PlayAttack();

	AC_Zombie();
};
