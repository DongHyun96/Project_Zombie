// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/C_BasicCharacter.h"
#include "Actor/Components/InteractionComponent/Interface/I_Interactable.h"
#include "GenericTeamAgentInterface.h"
#include "GlobalData.h" // TODO : FCursorItem curDraggedItem 때문에 넣었는데 문제 생기면 구조 바꿔야함.
#include "C_BasicPlayer.generated.h"


// TODO: PlayerState 랑 PlayerLifeState 를 하나로 통합할지...
// 캐릭터 상태
UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	Idle,
	Reviving,
	GettingUp,
	Dead,
};

// 이동 속도 결정 상태
UENUM(BlueprintType)
enum class EPlayerPoseState : uint8
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
class USphereComponent;
struct FInputActionValue;

UCLASS()
class PROJECT_ZOMBIE_API AC_BasicPlayer : public AC_BasicCharacter, public IGenericTeamAgentInterface, public II_Interactable
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

	// TurnInPlace 처리 담당 Actor Component (속도가 0인 상황에서, 왼쪽 오른쪽 회전 시 몸체 회전 모션 처리를 자연스럽게 도와준다)
	// 해당 기능은 PlayerCharacter만 처리를 할 예정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "PlayerAimComponent"))
	class UC_BasicPlayerAimComponent* m_BasicPlayerAimComponent;

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

	UPROPERTY(VisibleAnywhere, Category = "Components", meta = (DisplayName = "PoseColliderHandlerComponent"))
	class UC_PoseColliderHandlerComponent* m_PoseColliderHandlerComponent{};
	
	UPROPERTY(VisibleAnywhere, Category = "Components", meta = (DisplayName = "InteractionComponent"))
	class UC_InteractionComponent* m_InteractionComponent{};

	UPROPERTY(VisibleAnywhere, Category = "Components", meta = (DisplayName = "PlayerProfileComponent"))
	class UC_PlayerProfileComponent* m_PlayerProfileComponent{};
	
// [Status]
protected:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerState, VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	EPlayerState		m_PlayerState;
	
	UPROPERTY(ReplicatedUsing = OnRep_PlayerPoseState, VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	EPlayerPoseState	m_PlayerPoseState;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	EHandState			m_HandState;


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

	// 달리기 중 초당 부스트 소모량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float m_SprintBoostUseCost;

	// 달리지 않을 때 초당 부스트 회복량F
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float m_BoostRecoverCost;

	// 이 값 이상 회복되어야 다시 달리기 가능
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	//float m_MinBoostToSprint = 10.f;


// [Revive]
protected:
	// 플레이어와 상호작용할 수 있는 범위
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	USphereComponent* m_InteractionSphere;

	// 공중에서 죽었을 때 착지 후 사망 처리 여부
	bool m_IsPendingDead;

	// Strategy 로 뺌
	// 부활 후 플레이어가 일어나기까지 걸리는 시간 
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
	//float m_GetUpTime;

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
	
	// 현재 드래그된 아이템의 정보 구조체
	// TODO : C_DivideItemWidget에 CursorItem을 여기로 대체해야 하나?
	UPROPERTY()
	FCursorItem curDraggedItem{};

// [Timer]
private:
	// 플레이어의 LifeState 를 변경해서 BasicPlayer 가 관리
	FTimerHandle m_GetUpTimerHandle;

public:
	
	EPlayerState GetPlayerState() const { return m_PlayerState; }
	//void SetPlayerState(EPlayerState _NewState) { m_PlayerState = _NewState; }
	
	EPlayerPoseState GetPlayerMoveState() const { return m_PlayerPoseState; }
	void SetPlayerMoveState(EPlayerPoseState _MoveSpeedState) { m_PlayerPoseState = _MoveSpeedState; }

	EHandState GetHandState() const { return m_HandState; }
	void SetHandState(EHandState _HandState);

	UC_EquippedComponent* GetEquippedComponent() const { return m_EquippedComponent; }
	
	UC_TurnInPlaceComponent* GetTurnInPlaceComponent() const { return m_TurnInPlaceComponent; }

	UC_ControllerFSMComponent* GetControllerFSM() const { return m_ControllerFSMComponent; }
	
	UC_PingSystemComponent* GetPingSystemComponent() const { return m_PingSystemComponent; }

	UC_BasicPlayerAimComponent* GetAimComponent() const { return m_BasicPlayerAimComponent; }

	
	UC_PlayerProfileComponent* GetPlayerProfileComponent() const { return m_PlayerProfileComponent; }

	// interface II_Interactable 구현
	virtual UC_InteractionComponent* GetInteractionComponent() const override;

	FCursorItem GetCurDraggedItem() {return curDraggedItem;}
	
	// 드래그중인 아이템 관련 정보 저장
	bool SetCurDraggedItem(struct FInventoryEntry InEntry, UC_InvenComponent* SrcInvenComp, int32 SrcSlotIdx);

private:

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetHandState(EHandState _HandState);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetHandState(EHandState _HandState);
	
public:
	// curDraggedItem 초기화 및 서버에 Drag로 인한 잠금 해제 요청
	void ClearCurDraggedItem();
public:

	bool IsAlive() const { return m_PlayerState == EPlayerState::Idle; }
	bool IsReviving() const { return m_PlayerState == EPlayerState::Reviving; }
	bool IsGettingUp() const { return m_PlayerState == EPlayerState::GettingUp; }
	bool IsDead() const { return m_PlayerState == EPlayerState::Dead; }

	bool IsJumpInput() const { return m_IsJumpInput; }
	void SetIsJumpInput(bool _IsJumpInput) { m_IsJumpInput = _IsJumpInput; }
	
	bool IsFreeLook() const { return m_IsFreeLook; }
	void SetIsFreeLook(bool _IsFreeLook) { m_IsFreeLook = _IsFreeLook; }

	bool IsSprintInput() const { return m_IsSprintInput; }
	void SetIsSprintInput(bool _IsSprintInput) { m_IsSprintInput = _IsSprintInput; }

	bool IsCrouchTransitioning() const;

	bool IsSprinting() const { return m_PlayerPoseState == EPlayerPoseState::Sprint; }
	bool IsCrouching() const { return m_PlayerPoseState == EPlayerPoseState::Crouch; }


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


	// 캐릭터 그로기 처리
public:
	// 플레이어를 GettingUp 상태로 변경
	void StartGettingUp(float _GetUpDuration);

	// 구조 완료 후 GettingUp 상탤를 Idle 상태로 변경
	void FinishGettingUp();

	// 서버에 플레이어 상태 변경 요청
	void SetPlayerStateOnServer(EPlayerState _NewState);

	// Server함수 (생명 상태 변경)
protected:

	//UFUNCTION()
	void OnRep_PlayerState();

	// 체력이 0이 되어 Downed 상태로 변경
	UFUNCTION(Server, Reliable)
	void Server_EnterDownedState();

	// Server함수 (자세 변경)
protected:
	
	// 서버에서 자세 변경 처리 후 클라이언트에 리플리케이트
	UFUNCTION()
	void OnRep_PlayerPoseState();

	// 서버에 자세 변경 요청
	UFUNCTION(Server, Reliable)
	void Server_RequestSetPoseState(EPlayerPoseState _NewPoseState);

	// 서버에서 자세 변경 처리
	void SetPoseStateOnServer(EPlayerPoseState _NewPoseState);


	// Server함수 
public:
	// TODO : 퍼블릭으로 열어둬도 괜찮은가?
	// UI 드롭 시 서버에 안전하게 요청을 도달시켜 줄 확성기 RPC
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestMoveItem(UC_InvenComponent* SrcComp, int32 SrcIdx, UC_InvenComponent* DstComp, int32 DstIdx);

	// 드래그 시작
	UFUNCTION(Server, Reliable)
	void Server_RequestDragItemSlot(int32 SlotIndex, UC_InvenComponent* InteractedInven);
	
	UFUNCTION(Server, Reliable)
	void Server_CancelDragItemSlot(int32 SlotIndex, UC_InvenComponent* InteractedInven);
	
	// [경우 A] 슬롯에 떨어뜨렸을 때 (슬롯 간 분할 이동)
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestDivideMoveItem(
		UC_InvenComponent* SrcComp, int32 SrcIdx, 
		UC_InvenComponent* DstComp, int32 DstIdx, 
		int32 SplitCount
	);

	// [경우 B] 인벤토리 빈 곳에 떨어뜨렸을 때 (필드에 버리기/스폰)
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestDivideDropItem(
		UC_InvenComponent* SrcComp, int32 SrcIdx, 
		int32 SplitCount
	);
	
	// 슬롯 잠금 요청 RPC
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestLockSlot(UC_InvenComponent* TargetComp, int32 SlotIdx);

	// 슬롯 잠금 해제 요청 RPC
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestUnlockSlot(UC_InvenComponent* TargetComp, int32 SlotIdx);
	
protected:
	virtual void BeginPlay() override;
	
	//
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 리플리케이트 할 변수 등록
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


public:
	USpringArmComponent* GetSpringArm() { return m_SpringArm; }
	void SetSpringArmSocketOffset(FVector _SocketOffset);

	UCameraComponent* GetCamera() { return m_Camera; }
	void SetCameraFOV(float _FOV);

	USphereComponent* GetInteractionSphere() { return m_InteractionSphere; }

private:
	EPlayerPoseState DetermineMoveSpeedState() const;
	float GetMoveSpeedByState(EPlayerPoseState _MoveSpeedState) const;

	// 부스트 바 HUD를 갱신하는 함수
	void UpdateBoostBarHUD() const;

	// 웅크리기 상태 전환이 끝났을 때 실행되는 함수
	void OnPoseTransitionFinished(bool _bIsCrouched);

private:
	/// <summary>
	/// 입력 클라이언트가 자세 상태를 즉시 로컬에서 적용
	/// </summary>
	void ApplyPoseStateLocally(EPlayerPoseState _NewPoseState);

	/// <summary>
	/// 상태 적용(기본, 살리기, 그로기, 사망 등)
	/// </summary>
	void ApplyPlayerState();

	/// <summary>
	/// 속도 적용(달리기, 걷기, 웅크리기 등)
	/// </summary>
	void ApplyMovementSpeed();


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
