// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/C_BasicCharacter.h"
#include "GenericTeamAgentInterface.h"
#include "C_BasicPlayer.generated.h"

// 캐릭터 상태
UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	Idle,
	Dead,
};

// 이동 속도 결정 상태
UENUM(BlueprintType)
enum class EPlayerMoveSpeedState : uint8
{
	Walk,
	Sprint,
	Crouch,
	Aim,
};

// 손 상태
UENUM(BlueprintType)
enum class EHandState : uint8
{
	UnArmed,
	WeaponGun,
	WeaponMelee,
	WeaponThrowable,
	Max UMETA(Hidden)
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
class PROJECT_ZOMBIE_API AC_BasicPlayer : public AC_BasicCharacter, public IGenericTeamAgentInterface
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
	class UC_BasicPlayerInputComponent* m_PlayerInputComponent;
	
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

	// Player 상황 별, Controller rotation 처리 State machine 기능 담당
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "PlayerControllerFSMCom"))
	class UC_ControllerFSMComponent* m_ControllerFSMComponent{};

	UPROPERTY(VisibleAnywhere, Category = "Components", meta = (DisplayName = "PingSystemComponent"))
	class UC_PingSystemComponent* m_PingSystemComponent{};
	
// [Status]
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	EPlayerState		m_PlayerState;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	EPlayerMoveSpeedState	m_PlayerMoveSpeedState;

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

	// 최대 부스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
	float				m_MaxBoost;

	// 현재 부스트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float				m_CurBoost;


// 팀 설정
	FGenericTeamId		m_TeamId;


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
	float m_SprintSpeed;

	// 조준 시 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float m_AimMoveSpeed;

	// 웅크리기 시 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float m_CrouchSpeed;

	// 점프 입력 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool m_IsJumpInput;

	// 달리기 입력 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool m_IsSprintInput;

	/// 우선순위... 따로 enum으로 빼서 관리할까 
	/// 웅크리기 > 조준 > 달리기 > 일반 이동
	// 달리기 상태
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	//bool m_IsSprinting;

	//// 조준 상태
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	//bool m_IsAiming;

	//// 웅크리기 상태
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	//bool m_IsCrouching;


	// 달리기 중 초당 부스트 소모량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float m_SprintBoostUseCost;

	// 달리지 않을 때 초당 부스트 회복량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float m_BoostRecoverCost;

	// 이 값 이상 회복되어야 다시 달리기 가능
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	//float m_MinBoostToSprint = 10.f;

	// 웅크리기 전환 중인지 여부 (애니메이션 전환 중일 때 true)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool m_IsCrouchTransitioning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float m_CrouchTransitionStopTime;

	// 웅크리기 전환 타이머 핸들
	FTimerHandle m_CrouchTransitionTimerHandle;

private:
	// Free look 상태 (Hold Alt 상태)
	bool m_IsFreeLook{};

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
	
	EPlayerState GetPlayerState() const { return m_PlayerState; }
	void SetPlayerState(EPlayerState _NewState) { m_PlayerState = _NewState; }
	
	EPlayerMoveSpeedState GetPlayerMoveState() const { return m_PlayerMoveSpeedState; }
	void SetPlayerMoveState(EPlayerMoveSpeedState _MoveSpeedState) { m_PlayerMoveSpeedState = _MoveSpeedState; }

	EHandState GetHandState() const { return m_HandState; }
	void SetHandState(EHandState _HandState) { m_HandState = _HandState; }

	UC_EquippedComponent* GetEquippedComponent() const { return m_EquippedComponent; }
	
	UC_TurnInPlaceComponent* GetTurnInPlaceComponent() const { return m_TurnInPlaceComponent; }
	
	UC_PingSystemComponent* GetPingSystemComponent() const { return m_PingSystemComponent; }

public:
	bool IsDead() const { return m_IsDead; }
	void SetIsDead(bool _IsDead) { m_IsDead = _IsDead; }

	bool IsJumpInput() const { return m_IsJumpInput; }
	void SetIsJumpInput(bool _IsJumpInput) { m_IsJumpInput = _IsJumpInput; }
	
	bool IsFreeLook() const { return m_IsFreeLook; }
	void SetIsFreeLook(bool _IsFreeLook) { m_IsFreeLook = _IsFreeLook; }

	bool IsSprintInput() const { return m_IsSprintInput; }
	void SetIsSprintInput(bool _IsSprintInput) { m_IsSprintInput = _IsSprintInput; }

	bool IsCrouchTransitioning() const { return m_IsCrouchTransitioning; }

	bool IsSprinting() const { return m_PlayerMoveSpeedState == EPlayerMoveSpeedState::Sprint; }
	bool IsCrouching() const { return m_PlayerMoveSpeedState == EPlayerMoveSpeedState::Crouch; }


public:
	/// <summary>
	/// 캐릭터가 착지했을 때 실행되는 함수
	/// </summary>
	void Landed(const FHitResult& Hit) override;

	/// <summary>
	/// 후에 스탯 컴포넌트 쪽으로
	/// 부스트를 사용하고 HUD를 갱신한다.
	/// </summary>
	/// <param name="_UseAmount"> : 사용할 부스트 양 </param>
	/// <returns> : 사용 성공 여부 </returns>
	bool UseBoost(float _UseAmount);

	/// <summary>
	/// 후에 스탯 컴포넌트 쪽으로
	/// 부스트를 회복하고 HUD를 갱신한다.
	/// </summary>
	/// <param name="_RecoverAmount"> : 회복할 부스트 양 </param>
	void RecoverBoost(float _RecoverAmount);

	/// <summary>
	/// 달리기 시작
	/// </summary>
	void StartSprint();
	
	/// <summary>
	///	달리기 종료
	/// </summary>
	void StopSprint();

	/// <summary>
	/// 웅크리기 토글
	/// </summary>
	void ToggleCrouch();

	/// <summary>
	/// 속도 적용(달리기, 걷기, 웅크리기 등)
	/// </summary>
	void ApplyMovementSpeed();
	
	void ApplyCrouchSpeed();
	void ApplyWalkSpeed();
	
public:
	// UI 드롭 시 서버에 안전하게 요청을 도달시켜 줄 확성기 RPC
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestMoveItem(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp, int32 DstIdx);

protected:
	virtual void BeginPlay() override;

private:
	EPlayerMoveSpeedState DetermineMoveSpeedState() const;
	float GetMoveSpeedByState(EPlayerMoveSpeedState _MoveSpeedState) const;

	// 부스트 바 HUD를 갱신하는 함수
	void UpdateBoostBarHUD() const;

public:
	virtual void Tick(float DeltaTime) override;

	// Cotroller가 빙의할 때 실행되는 함수.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// 데미지 처리 함수 
	virtual float TakeDamage(float _Damage, struct FDamageEvent const& _DamageEvent
		, class AController* _InstigatorController, AActor* _InstigatorActor) override;

	// TeamAgentInterface
public:
	virtual void SetGenericTeamId(const FGenericTeamId& _NewId) override { m_TeamId = _NewId; }
	virtual FGenericTeamId GetGenericTeamId() const override { return m_TeamId; }
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& _Other) const override;

	
public:
	class UC_InvenComponent* GetInvenComponent() { return m_InvenComponent; }
public:
	AC_BasicPlayer();
};
