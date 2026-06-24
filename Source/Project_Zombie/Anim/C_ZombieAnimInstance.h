// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Anim/C_BasicCharacterAnimInstance.h"
#include "C_ZombieAnimInstance.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API UC_ZombieAnimInstance : public UC_BasicCharacterAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCharacterMovementComponent*	m_MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class AC_Zombie*					m_Zombie;

	// 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float	m_GroundSpeed;

	// 방향
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float	m_Direction;

	// 수직 속도 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float	m_VerticalSpeed;
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float _DT) override;

};
