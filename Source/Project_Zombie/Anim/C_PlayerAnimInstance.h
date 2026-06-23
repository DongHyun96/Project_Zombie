// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_BasicCharacterAnimInstance.h"
#include "C_PlayerAnimInstance.generated.h"

//class AC_BasicPlayer;

enum class EHandState : uint8;

UCLASS()
class PROJECT_ZOMBIE_API UC_PlayerAnimInstance : public UC_BasicCharacterAnimInstance
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCharacterMovementComponent* m_MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class AC_BasicPlayer* m_Character;

	// HandState
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Basic State")
	EHandState m_HandState;
	
	// 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float	m_GroundSpeed;

	// 방향
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float	m_Direction;

	// 낙하
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool	m_IsFall;

	// 수직 속도 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float	m_VerticalSpeed;

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float _DT) override;
};
