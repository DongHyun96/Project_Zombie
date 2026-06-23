// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/C_BasicCharacter.h"
#include "C_BasicPlayer.generated.h"

// 캐릭터 상태
UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	Idle,
	Dead,
};

// 손 상태
UENUM(BlueprintType)
enum class EHandState : uint8
{
	UnArmed,
	WeaponGun,
	WeaponMelee,
	WeaponThrowable,
};

// 시점 상태
UENUM(BlueprintType)
enum class EPlayerViewMode : uint8
{
	TPS,	// 3인칭
	FPS,	// 1인칭
};


class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class PROJECT_ZOMBIE_API AC_BasicPlayer : public AC_BasicCharacter
{
	GENERATED_BODY()
	
// [Component]
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "SpringArm"))
	class USpringArmComponent* m_SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "MainCamera"))
	class UCameraComponent* m_Camera;

	// 새로 추가된 우리만의 커스텀 인풋 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "PlayerInput"))
	class UC_BasicPlayerInputComponent* m_InputComponent;


// [Status]
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	EPlayerState		m_PlayerState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	bool				m_IsDead;


	// => 여기서부터는 나중에 StatComponent으로 분리?
	// 기본 이동 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	float				m_BaseMaxSpeed;

	// 최대 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float				m_MaxHP;

	// 현재 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float				m_CurHP;


// [Camera]
protected:
	// 시점 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	EPlayerViewMode		m_PlayerViewMode;

	// 3인칭 카메라 암 길이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float				m_TPSCameraArmLength;

	// 1인칭 카메라 암 길이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float				m_FPSCameraArmLength;

	// => 나중에 InputComponent으로 분리?
	// 마우스 감도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float 				m_MouseSensitivity;


// [Movement]
protected:
	// 걷기 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float m_WalkSpeed;

	// 달리기 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float m_RunSpeed;

	// 조준 시 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float m_AimMoveSpeed;

	// 웅크리기 시 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float m_CrouchSpeed;

	// 점프 입력 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool m_IsJumpInput;

	/// 우선순위... 따로 enum으로 빼서 관리할까 
	/// 웅크리기 > 조준 > 달리기 > 일반 이동
	// 달리기 상태
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool m_IsRunning;

	// 조준 상태
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool m_IsAiming;

	// 웅크리기 상태
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool m_IsCrouching;


// [Weapon]
protected:


// [Inventory]
protected:
	// => 나중에 InventoryComponent으로 분리?
	// 상호작용 가능
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	bool m_IsCanInteract;

	// 인벤토리 열려있나
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	bool m_IsInventoryOpen;


public:
	UFUNCTION(BlueprintCallable)
	EPlayerState GetPlayerState() const { return m_PlayerState; }
	void SetPlayerState(EPlayerState _NewState) { m_PlayerState = _NewState; }

	bool IsDead() const { return m_IsDead; }
	void SetIsDead(bool _IsDead) { m_IsDead = _IsDead; }

	bool IsJumpInput() const { return m_IsJumpInput; }
	void SetIsJumpInput(bool _IsJumpInput) { m_IsJumpInput = _IsJumpInput; }

public:
	// 캐릭터가 착지했을 때 실행되는 함수
	void Landed(const FHitResult& Hit) override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Cotroller가 빙의할 때 실행되는 함수.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// 데미지 처리 함수 
	virtual float TakeDamage(float _Damage, struct FDamageEvent const& _DamageEvent
		, class AController* _InstigatorController, AActor* _InstigatorActor) override;

public:
	AC_BasicPlayer();
};
