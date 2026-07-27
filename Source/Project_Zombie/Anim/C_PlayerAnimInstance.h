// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_BasicCharacterAnimInstance.h"
#include "C_PlayerAnimInstance.generated.h"

//class AC_BasicPlayer;

enum class EPlayerState : uint8;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Basic State")
	EPlayerState m_PlayerState;

	// HandState
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Basic State")
	EHandState m_HandState;

	// 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float	m_GroundSpeed;

	// 방향
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float	m_Direction;

	// 점프 (점프 키를 눌러서 점프)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool	m_IsJumpInput;

	// 낙하
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool	m_IsFall;

	// 수직 속도 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float	m_VerticalSpeed;

	// 웅크리기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool	m_IsCrouch;

	// 플레이어 부활 중인지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Revive")
	bool	m_IsRevivingPlayer;

	// 에임 오프셋용 피치 값
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AimOffset")
	float	m_Pitch;

	// 에임 오프셋용 야 값
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AimOffset")
	float	m_Yaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AimOffset")
	float m_Alpha = 1.0f;

	// 애님그래프(AnimGraph)에서 참조할 왼손 IK Transform 변수
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FVector m_LeftHandIKTransform;

	// 애님그래프(AnimGraph)에서 참조할 오른손 IK Transform 변수
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FVector m_RightHandIKTransform;

	// ADS시 오른손 애니메이션 알파
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	float HandIKAlpha;

	

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float _DT) override;
};
