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

// 이동 상태
UENUM(BlueprintType)
enum class EPlayerMoveState : uint8
{
	Stand,
	Crouch,
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "EquippedComponent"))
	class UC_EquippedComponent* m_EquippedComponent{};


	// TurnInPlace 처리 담당 Actor Component (속도가 0인 상황에서, 왼쪽 오른쪽 회전 시 몸체 회전 모션 처리를 자연스럽게 도와준다)
	// 해당 기능은 PlayerCharacter만 처리를 할 예정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "TurnInPlaceComponent"))
	class UC_TurnInPlaceComponent* m_TurnInPlaceComponent{};

	/// <summary>
	/// InvenComponent,
	/// 플레이어의 아이템을 소유하는 컴포넌트.
	/// </summary>
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "InvenComponent"))
	class UC_InvenComponent* m_InvenComponent{};
	
// [Status]
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	EPlayerState		m_PlayerState;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	EPlayerMoveState	m_PlayerMoveState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	EHandState			m_HandState;

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

	// 웅크리기 전환 중인지 여부 (애니메이션 전환 중일 때 true)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool m_IsCrouchTransitioning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float m_CrouchTransitionStopTime;

	// 웅크리기 전환 타이머 핸들
	FTimerHandle m_CrouchTransitionTimerHandle;


// [Weapon] - EquippedComponent에서 관리할 예정
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
	UFUNCTION(BlueprintPure)
	EPlayerState GetPlayerState() const { return m_PlayerState; }
	void SetPlayerState(EPlayerState _NewState) { m_PlayerState = _NewState; }
	
	EPlayerMoveState GetPlayerMoveState() const { return m_PlayerMoveState; }
	void SetPlayerMoveState(EPlayerMoveState _MoveState) { m_PlayerMoveState = _MoveState; }

	EHandState GetHandState() const { return m_HandState; }
	void SetHandState(EHandState _HandState) { m_HandState = _HandState; }

	UC_EquippedComponent* GetEquippedComponent() const { return m_EquippedComponent; }
	
	UFUNCTION(BlueprintPure)
	UC_TurnInPlaceComponent* GetTurnInPlaceComponent() const { return m_TurnInPlaceComponent; }

public:
	bool IsDead() const { return m_IsDead; }
	void SetIsDead(bool _IsDead) { m_IsDead = _IsDead; }

	bool IsJumpInput() const { return m_IsJumpInput; }
	void SetIsJumpInput(bool _IsJumpInput) { m_IsJumpInput = _IsJumpInput; }

	bool IsCrouching() const { return m_IsCrouching; }
	void SetIsCrouching(bool _IsCrouching) { m_IsCrouching = _IsCrouching; }

public:
	// 캐릭터가 착지했을 때 실행되는 함수
	void Landed(const FHitResult& Hit) override;

	// 웅크리기 토글
	void ToggleCrouch();

	// 웅크리기 전환 완료 후 속도 적용
	void ApplyCrouchSpeed();
	void ApplyWalkSpeed();

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
