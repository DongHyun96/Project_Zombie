// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Character/Player/C_BasicPlayer.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputSubsystems.h"
#include "../../Components/C_BasicPlayerInputComponent.h"
#include "C_BasicPlayer.h"

#include "Actor/Components/C_ControllerFSMComponent.h"
#include "Actor/Components/C_EquippedComponent.h"
#include "Actor/Components/C_TurnInPlaceComponent.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Actor/Components/C_PingSystemComponent.h"
#include "Actor/Components/C_BasicPlayerAimComponent.h"
#include "Actor/Components/C_PoseColliderHandlerComponent.h"
#include "Actor/Components/InteractionComponent/C_InteractionComponent.h"

#include "GameFramework/PlayerState.h"
#include "Actor/Components/C_PlayerStatComponent.h"
#include "Controller/C_BasicPlayerController.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "GameModeAndManager/C_UIManager.h"

#include "UI/InvenUI/C_InventoryGridWidget.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/InvenUI/Equipment/C_EquipmentWidget.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Utility/C_Util.h"

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

#include "TimerManager.h"

#include "Net/UnrealNetwork.h"

AC_BasicPlayer::AC_BasicPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// Replication 설정
	bReplicates = true;
	SetReplicateMovement(true);
	
	m_SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	m_SpringArm->SetupAttachment(RootComponent);
	m_SpringArm->TargetArmLength = 250.0f;
	m_SpringArm->bUsePawnControlRotation = true;

	m_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	m_Camera->SetupAttachment(m_SpringArm);

	m_InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	m_InteractionSphere->SetupAttachment(RootComponent);
	m_InteractionSphere->SetSphereRadius(150.0f);


	// 캐릭터 상태 초기화
	m_PlayerLifeState = EPlayerLifeState::Alive;
	m_PlayerPoseState = EPlayerPoseState::Walk;

	// 부활 중인지 여부 초기화
	m_IsRevivingPlayer = false;
	m_RevivingPlayer = nullptr;

	// 점프높이 설정
	GetCharacterMovement()->JumpZVelocity = 600.f;

	// 이동 속도 설정
	m_WalkSpeed = 300.f;
	m_SprintSpeed = 600.f;
	m_CrouchSpeed = 200.f;

	m_BaseMaxSpeed = m_WalkSpeed;

	GetCharacterMovement()->MaxWalkSpeed = m_WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = m_CrouchSpeed;

	// 달리기 입력 초기화
	m_IsSprintInput = false;

	// 부스트 초기화
	m_MaxBoost = 100.f;
	m_CurBoost = m_MaxBoost;

	// 부스트 사용량 설정
	m_SprintBoostUseCost = 20.f;
	m_BoostRecoverCost = 15.f;

	// 점프 입력 초기화
	m_IsJumpInput = false;

	// TeamId
	SetGenericTeamId((uint8)ETeamType::Player);

	// 우리가 만든 InputComponent 클래스를 Player에게 추가.
	m_PlayerInputComponent = CreateDefaultSubobject<UC_BasicPlayerInputComponent>(TEXT("PlayerInputComponent"));

	m_EquippedComponent = CreateDefaultSubobject<UC_EquippedComponent>(TEXT("EquippedComponent"));
	
	// TurnInPlace Component
	m_TurnInPlaceComponent = CreateDefaultSubobject<UC_TurnInPlaceComponent>(TEXT("TurnInPlaceComponent"));

	m_InvenComponent = CreateDefaultSubobject<UC_InvenComponent>(TEXT("InvenComponent"));
	

	
	// ControllerFSM Component
	m_ControllerFSMComponent = CreateDefaultSubobject<UC_ControllerFSMComponent>(TEXT("ControllerFSMComponent"));
	
	// PingSystem Component
	m_PingSystemComponent = CreateDefaultSubobject<UC_PingSystemComponent>(TEXT("PingSystemComponent"));

	// PlayerAim Component
	m_BasicPlayerAimComponent = CreateDefaultSubobject<UC_BasicPlayerAimComponent>(TEXT("PlayerAimComponent"));
	
	m_StatComponent = CreateDefaultSubobject<UC_PlayerStatComponent>(TEXT("StatComponent"));
	
	// PoseColliderHandler Component
	m_PoseColliderHandlerComponent = CreateDefaultSubobject<UC_PoseColliderHandlerComponent>(TEXT("PoseColliderHandlerComponent"));

	// Interaction Component
	//m_InteractionComponent = CreateDefaultSubobject<UC_InteractionComponent>(TEXT("InteractionComponent"));
}


void AC_BasicPlayer::BeginPlay()
{
	Super::BeginPlay();

	// GameLevelManager에 해당 Player 등록
	if (UC_GameLevelManager* LevelManager = GetWorld()->GetSubsystem<UC_GameLevelManager>())
		LevelManager->AddPlayer(this);
	

	// 웅크리기 완료 시 호출할 OnPoseTransitionFinished 바인딩
	if (m_PoseColliderHandlerComponent)
	{
		m_PoseColliderHandlerComponent
			->OnPoseTransitionFinished.AddUObject(this, &AC_BasicPlayer::OnPoseTransitionFinished);
	}

	UpdateBoostBarHUD();

	// InventoryWidget에 Player의 InvenComponent 초기화 및 델리게이트 진행
	APlayerController* PC = Cast<APlayerController>(GetController());
	
	if (!PC) return;
	
	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	
	if (!UIManager) return;
	
	UIManager->GetInventoryWidget()->GetPlayerGridWidget()->SetInvenComponent(m_InvenComponent);
	
	UIManager->GetInventoryWidget()->GetEquipmentWidget()->InitEquipmentWidget(m_InvenComponent);
	// 까지
	

	// 플레이어의 인벤에 장비 전용 인덱스 추가.
	//m_InvenComponent->SetMaxSlots(45 + static_cast<int32>(EWeaponSlot::Max));
	
	// 웅크리기 완료 시 호출할 OnPoseTransitionFinished 바인딩
	if (m_PoseColliderHandlerComponent)
	{
		m_PoseColliderHandlerComponent
			->OnPoseTransitionFinished.AddUObject(this, &AC_BasicPlayer::OnPoseTransitionFinished);
	}


	// 입력 시스템 초기화
	//InitInput();
}

void AC_BasicPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	// 팅기거나 접속을 종료하면 드래그하고 있던 아이템 잠금 해제.
	Server_CancelDragItemSlot(curDraggedItem.SourceSlotIndex, curDraggedItem.SourceInvenComp);
}

void AC_BasicPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ======= 리플리케이트 하고싶은 맴버를 등록 ==========
	// 자세 상태 
	DOREPLIFETIME(AC_BasicPlayer, m_PlayerPoseState);

	// 플레이어 생존 상태 
	DOREPLIFETIME(AC_BasicPlayer, m_PlayerLifeState);

	// 다른 플레이어 구조 중인지 
	DOREPLIFETIME(AC_BasicPlayer, m_IsRevivingPlayer);

	// 구조 중인 플레이어
	DOREPLIFETIME(AC_BasicPlayer, m_RevivingPlayer);
}

void AC_BasicPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/// 나중에 스탯 컴포넌트로 분리할 예정
	// 달리기 중이면 부스트 소모
	if (m_PlayerPoseState == EPlayerPoseState::Sprint)
	{
		UseBoost(m_SprintBoostUseCost * DeltaTime);
		
		if (m_CurBoost <= 0.f)
		{
			StopSprint();
		}
	}
	// 달리기 중이 아니면 부스트 회복
	else
	{
		RecoverBoost(m_BoostRecoverCost * DeltaTime);
	}

	// [Aim] 카메라 변환 중일 때만 함수 호출
	if (m_BasicPlayerAimComponent->IsTransitioningCamera())
	{
		m_BasicPlayerAimComponent->UpdateCameraInterpolation(DeltaTime);
	}

}

void AC_BasicPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (m_PlayerInputComponent)
	{
		m_PlayerInputComponent->InitializePlayerInput(PlayerInputComponent, this);
	}
}

float AC_BasicPlayer::TakeDamage(float _Damage, FDamageEvent const& _DamageEvent, AController* _InstigatorController, AActor* _InstigatorActor)
{
	UC_Util::Print("Player Damaged", FColor::MakeRandomColor(), 10.f);

	const float DamageAmount = Super::TakeDamage(_Damage, _DamageEvent, _InstigatorController, _InstigatorActor);
	return DamageAmount;
}

bool AC_BasicPlayer::SetCurDraggedItem(struct FInventoryEntry InEntry, UC_InvenComponent* SrcInvenComp, int32 SrcSlotIdx)
{
	// 매개변수중 하나라도 유효하지 않다면 false가 리턴됨. curDraggedItem 초기화.
	if (curDraggedItem.SetCursorItem(InEntry, SrcInvenComp, SrcSlotIdx)) return true;
	
	// TODO : 만약 false가 리턴되어도 curDraggedItem이 초기화되면 안되는 상황이 존재한다면?
	curDraggedItem.Clear();
	
	return false;
}

void AC_BasicPlayer::ClearCurDraggedItem()
{
	// 해당 슬롯의 아이템의 잠금 상태를 해제 요청.
	Server_CancelDragItemSlot(curDraggedItem.SourceSlotIndex, curDraggedItem.SourceInvenComp);
	
	curDraggedItem.Clear();
}

bool AC_BasicPlayer::IsCrouchTransitioning() const
{
	return m_PoseColliderHandlerComponent->IsTransitioning();
}

void AC_BasicPlayer::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	m_IsJumpInput = false;
}

bool AC_BasicPlayer::UseBoost(float _UseAmount)
{
	if (_UseAmount <= 0.f || m_MaxBoost <= 0.f || m_CurBoost <= 0.f)
		return false;


	// =================임시용==========================
	/*if (m_CurBoost >= 0.f)
	{
		Die();
		return false;
	}*/
	// ================================================

	float PrevBoost = m_CurBoost;
	bool bHasBoost = (m_CurBoost >= _UseAmount);

	if (bHasBoost)
	{	
		// 부스트 사용 
		m_CurBoost = FMath::Max(0.f, m_CurBoost - _UseAmount);

		// 값이 변경되었을 경우 HUD 업데이트
		if (!FMath::IsNearlyEqual(PrevBoost, m_CurBoost))
			UpdateBoostBarHUD();
	}
	
	return bHasBoost;
}

void AC_BasicPlayer::RecoverBoost(float _RecoverAmount)
{
	if (_RecoverAmount <= 0.f || m_MaxBoost <= 0.f || m_CurBoost >= m_MaxBoost)
		return;

	float PrevBoost = m_CurBoost;
	
	// 부스트 회복
	m_CurBoost = FMath::Min(m_MaxBoost, m_CurBoost + _RecoverAmount);

	// 값이 변경되었을 경우 HUD 업데이트
	if (!FMath::IsNearlyEqual(PrevBoost, m_CurBoost))
		UpdateBoostBarHUD();
}

void AC_BasicPlayer::StartSprint()
{
	// 공중일 때는 달리기 불가
	if (!GetCharacterMovement() || GetCharacterMovement()->IsFalling())
		return;

	// 자세 전환 중에는 달리기 불가
	if (m_PoseColliderHandlerComponent->IsTransitioning())
		return;

	// 웅크리기 중이거나 웅크리기 전환 중일 때는 달리기 불가
	if (m_PlayerPoseState == EPlayerPoseState::Crouch)
		return;

	// 부스트가 없으면 달리기 불가
	if (m_CurBoost <= 0.f)
		return;

	m_IsSprintInput = true;

	m_PlayerPoseState = EPlayerPoseState::Sprint;

	ApplyMovementSpeed();
}

void AC_BasicPlayer::StopSprint()
{
	m_IsSprintInput = false;

	// 이미 다른 자세로 변경되었다면 Walk 로 변경하지 않음
	if (m_PlayerPoseState != EPlayerPoseState::Sprint)
		return;

	m_PlayerPoseState = EPlayerPoseState::Walk;

	ApplyMovementSpeed();
}

void AC_BasicPlayer::ToggleCrouch()
{
	if (GetCharacterMovement()->IsFalling())
		return;

	if (m_PoseColliderHandlerComponent->IsTransitioning())
		return;

	// 달리기 중 웅크리기를 누르면 달리기 입력부터 해제
	if (m_PlayerPoseState == EPlayerPoseState::Sprint)
	{
		m_IsSprintInput = false;
	}

	const EPlayerPoseState NewPoseState
		= m_PlayerPoseState == EPlayerPoseState::Crouch ? EPlayerPoseState::Walk : EPlayerPoseState::Crouch;


	// 서버가 한거면 바로 PoseState를 변경
	if (HasAuthority())
	{
		// Replicated 변수이므로 서버에서 변경 시 클라이언트에 자동으로 동기화됨
		// 그래서 Multicast를 사용하지 않고 바로 서버에서 변경
		SetPoseStateOnServer(NewPoseState);
	}
	// 클라이언트가 한거면 서버에 요청
	else
	{
		Server_RequestSetPoseState(NewPoseState);
	}
}

/// <summary>
///	나중에 enum으로 상태를 관리하는 방식으로 변경할 듯
/// </summary>
void AC_BasicPlayer::ApplyMovementSpeed()
{
	if (!GetCharacterMovement())
		return;

	if (IsDead())
	{
		GetCharacterMovement()->MaxWalkSpeed = 0.f;
		return;
	}

	// 웅크리기 전환 중일 때는 잠깐 정지
	if (m_PoseColliderHandlerComponent->IsTransitioning())
	{
		GetCharacterMovement()->MaxWalkSpeed = 0.f;
		return;
	}

	// 상태에 따른 이동 속도 적용
	switch (m_PlayerPoseState)
	{
	case EPlayerPoseState::Walk:
		GetCharacterMovement()->MaxWalkSpeed = m_WalkSpeed;
		break;
	case EPlayerPoseState::Sprint:
		GetCharacterMovement()->MaxWalkSpeed = m_SprintSpeed;
		break;
	case EPlayerPoseState::Crouch:
		GetCharacterMovement()->MaxWalkSpeed = m_CrouchSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = m_CrouchSpeed;
		break;
	case EPlayerPoseState::Aim:
		//GetCharacterMovement()->MaxWalkSpeed = m_AimSpeed;
		break;
	default:
		GetCharacterMovement()->MaxWalkSpeed = m_WalkSpeed;
		break;
	}
}

void AC_BasicPlayer::Die()
{
	if (IsDead())
		return;

	GetStatComponent()->SetCurHP(0.f);

	m_PlayerState = EPlayerState::Dead;
	m_IsSprintInput = false;

	// 이동 중지

	// 사망 시 다른 플레이어가 살려줄 수 있도록 
}


void AC_BasicPlayer::UpdateBoostBarHUD() const
{
	if (APlayerController* PC = GetController<APlayerController>())
	{
		if (AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD()))
		{
			if (UC_GameMainHUD* MainHUD = UIManager->GetMainHUDWidget())
				MainHUD->UpdateBoostBar(m_CurBoost, m_MaxBoost);
		}
	}
}


void AC_BasicPlayer::OnPoseTransitionFinished(bool _bIsCrouched)
{
	m_PlayerPoseState = _bIsCrouched ? EPlayerPoseState::Crouch : EPlayerPoseState::Walk;

	// 웅크리기 전환 완료 후 이동 속도 갱신
	ApplyMovementSpeed();
}

ETeamAttitude::Type AC_BasicPlayer::GetTeamAttitudeTowards(const AActor& _Other) const
{
	const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(&_Other);

	if (TeamAgent)
	{
		FGenericTeamId OtherID = TeamAgent->GetGenericTeamId();

		if (GetGenericTeamId() == OtherID)
		{
			return ETeamAttitude::Friendly;
		}
		else
		{
			return ETeamAttitude::Hostile;
		}
	}

	// 팀 설정 기능이 없는 Actor 인 경우 중립
	return ETeamAttitude::Neutral;
}

void AC_BasicPlayer::OnRep_PlayerLifeState()
{
}

void AC_BasicPlayer::OnRep_IsRevivingPlayer()
{
}

void AC_BasicPlayer::StartGettingUpOnServer(float _GetUpDuration)
{
	// 서버에서만 처리
	if (!HasAuthority())
		return;

	// Downed 상태가 아니면 일어날 필요없음
	if (m_PlayerLifeState != EPlayerLifeState::Downed)
		return;

	// Timer 중복 방지 // 이미 돌아가고 있었으면 Timer 제거
	GetWorldTimerManager().ClearTimer(m_GetUpTimerHandle);

}

void AC_BasicPlayer::OnRep_PlayerPoseState()
{
	if (!m_PoseColliderHandlerComponent)
		return;

	const bool bWantsToCrouch = m_PlayerPoseState == EPlayerPoseState::Crouch;

	m_PoseColliderHandlerComponent->StartCrouchTransition(bWantsToCrouch);

	ApplyMovementSpeed();
}


void AC_BasicPlayer::SetPoseStateOnServer(EPlayerPoseState _NewPoseState)
{
	if (!HasAuthority())
		return;

	if (m_PlayerPoseState == _NewPoseState)
		return;

	if (GetCharacterMovement()->IsFalling())
		return;

	const bool bWantsToCrouch = _NewPoseState == EPlayerPoseState::Crouch;

	const bool bStart = m_PoseColliderHandlerComponent->SetCrouched(bWantsToCrouch);

	if (!bStart)
		return;

	m_PlayerPoseState = _NewPoseState;

	// 자세 전환 중에 잠깐 멈춤
	ApplyMovementSpeed();

	// 상태를 가능한 빨리 변경
	// ForceNetUpdate();
}

void AC_BasicPlayer::Server_RequestSetPoseState_Implementation(EPlayerPoseState _NewPoseState)
{
	SetPoseStateOnServer(_NewPoseState);
}


bool AC_BasicPlayer::Server_RequestMoveItem_Validate(UC_InvenComponent* SrcComp, int32 SrcIdx,
	UC_InvenComponent* DstComp, int32 DstIdx)
{
	// null 체크 (가장 먼저 수행)
	if (!SrcComp || !DstComp) return false;

	// 동일 슬롯 체크 (제자리 드롭 방지)
	if (SrcComp == DstComp && SrcIdx == DstIdx) return false;

	// 해당 슬롯의 유효성 검사
	if (!SrcComp->GetInventoryItems().IsValidIndex(SrcIdx) || !DstComp->GetInventoryItems().IsValidIndex(DstIdx)) return false;

	return true;
}

void AC_BasicPlayer::Server_RequestMoveItem_Implementation(UC_InvenComponent* SrcComp, int32 SrcIdx,
	UC_InvenComponent* DstComp, int32 DstIdx)
{
	APlayerController* pPC = Cast<APlayerController>( GetController());
	
	if (!pPC || !pPC->PlayerState) return;

	// 현재 요청을 보낸 플레이어의 고유 ID 추출
	int32 PlayerId = pPC->PlayerState->GetPlayerId();
	
	SrcComp->ProcessItemMove(SrcComp, SrcIdx, DstComp, DstIdx, PlayerId);
}

void AC_BasicPlayer::Server_RequestDragItemSlot_Implementation(int32 SlotIndex, UC_InvenComponent* InteractedInven)
{
	AC_BasicPlayerController* pPC = Cast<AC_BasicPlayerController>( GetController());
	
	if (!pPC) return;
	
	pPC->Server_ActiveDraggedInven = InteractedInven;
	pPC->Server_ActiveDraggedSlotIndex = SlotIndex;
	InteractedInven->StartDragItemSlot(SlotIndex, pPC->GetPlayerState<APlayerState>()->GetPlayerId()); 
}

void AC_BasicPlayer::Server_CancelDragItemSlot_Implementation(int32 SlotIndex, UC_InvenComponent* InteractedInven)
{
	APlayerController* pPC = Cast<APlayerController>( GetController());
	
	if (!pPC) return;
	
	InteractedInven->CancelDragItemSlot(SlotIndex, pPC->GetPlayerState<APlayerState>()->GetPlayerId());
}

bool AC_BasicPlayer::Server_RequestDivideMoveItem_Validate(UC_InvenComponent* SrcComp, int32 SrcIdx,
	UC_InvenComponent* DstComp, int32 DstIdx, int32 SplitCount)
{
	return true;
}

void AC_BasicPlayer::Server_RequestDivideMoveItem_Implementation(UC_InvenComponent* SrcComp, int32 SrcIdx,
	UC_InvenComponent* DstComp, int32 DstIdx, int32 SplitCount)
{
	APlayerController* pPC = Cast<APlayerController>(GetController());
	if (!pPC || !pPC->PlayerState || SplitCount <= 0) return;

	int32 PlayerId = pPC->PlayerState->GetPlayerId();
    
	// 컴포넌트에 분할 이동 처리 요청
	SrcComp->ProcessItemDivideMove(SrcComp, SrcIdx, DstComp, DstIdx, SplitCount, PlayerId);
}

bool AC_BasicPlayer::Server_RequestDivideDropItem_Validate(UC_InvenComponent* SrcComp, int32 SrcIdx, int32 SplitCount)
{
	return true;
}

void AC_BasicPlayer::Server_RequestDivideDropItem_Implementation(UC_InvenComponent* SrcComp, int32 SrcIdx,
	int32 SplitCount)
{
	if (!SrcComp || SplitCount <= 0) return;

	APlayerController* pPC = Cast<APlayerController>(GetController());
	if (!pPC || !pPC->PlayerState) return;

	int32 PlayerId = pPC->PlayerState->GetPlayerId();

	FInventoryEntry TempEntry = SrcComp->GetItemAt(SrcIdx);
	
	FName TempName = TempEntry.ItemRowName;
	
	if (TempName == NAME_None) return;
	
	// 1. 인벤토리 컴포넌트에게 데이터를 처리하고 실제로 드롭할 개수를 받아옵니다.
	// (검증 실패 시 알아서 0을 반환하며 인벤토리 데이터는 건드리지 않습니다)
	int32 ActualDropCount = SrcComp->ProcessItemDivideDrop(SrcIdx, SplitCount, PlayerId);
    
	if (ActualDropCount <= 0) return;

	// 2. 안전하게 깎인 게 확인되었으므로, 서버 월드에 실제 아이템 액터 스폰
	UC_ItemManager* ItemManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	if (ItemManager) ItemManager->DropItemByPlayer(TempName, ActualDropCount, this);
}

bool AC_BasicPlayer::Server_RequestLockSlot_Validate(UC_InvenComponent* TargetComp, int32 SlotIdx)
{
	return true;
}


void AC_BasicPlayer::Server_RequestLockSlot_Implementation(UC_InvenComponent* TargetComp, int32 SlotIdx)
{
	if (!TargetComp) return;
    
	APlayerController* pPC = Cast<APlayerController>(GetController());
	if (!pPC || !pPC->PlayerState) return;

	int32 PlayerId = pPC->PlayerState->GetPlayerId();
    
	// 이미 다른 사람이 잠그고 있는 상태가 아니라면 잠금 승인
	const FInventoryEntry& Entry = TargetComp->GetItemAt(SlotIdx);
	if (Entry.LockedByPlayerID == INDEX_NONE)
	{
		TargetComp->SetSlotLockState(SlotIdx, PlayerId);
	}
}

bool AC_BasicPlayer::Server_RequestUnlockSlot_Validate(UC_InvenComponent* TargetComp, int32 SlotIdx)
{
	return true;
}

void AC_BasicPlayer::Server_RequestUnlockSlot_Implementation(UC_InvenComponent* TargetComp, int32 SlotIdx)
{
	if (!TargetComp) return;
    
	APlayerController* pPC = Cast<APlayerController>(GetController());
	if (!pPC || !pPC->PlayerState) return;

	int32 PlayerId = pPC->PlayerState->GetPlayerId();

	// 내가 잠근 슬롯인 경우에만 해제 허용 (보안 검증)
	const FInventoryEntry& Entry = TargetComp->GetItemAt(SlotIdx);
	if (Entry.LockedByPlayerID == PlayerId)
	{
		TargetComp->SetSlotLockState(SlotIdx, INDEX_NONE);
	}
}

void AC_BasicPlayer::SetSpringArmSocketOffset(FVector _SocketOffset)
{
	if(m_SpringArm)
		m_SpringArm->SocketOffset = _SocketOffset;
}

void AC_BasicPlayer::SetCameraFOV(float _FOV)
{
	if(m_Camera)
		m_Camera->FieldOfView = _FOV;
}

EPlayerPoseState AC_BasicPlayer::DetermineMoveSpeedState() const
{
	return EPlayerPoseState();
}

float AC_BasicPlayer::GetMoveSpeedByState(EPlayerPoseState _MoveSpeedState) const
{
	return 0.0f;
}

//void AC_BasicPlayer::InitInput()
//{
//	APlayerController* PC = Cast<APlayerController>(GetController());
//	if (!PC)
//		return;
//
//	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
//	if (!LocalPlayer)
//		return;
//
//	UEnhancedInputLocalPlayerSubsystem* Subsystem =
//		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
//
//	Subsystem->ClearAllMappings();
//
//	if (Subsystem && DefaultMappingContext)
//	{
//		Subsystem->AddMappingContext(DefaultMappingContext, 0);
//	}
//}

//
//void AC_BasicPlayer::MoveAction(const FInputActionValue& Value)
//{
//	if (GetController() != nullptr)
//	{
//		FVector2D Input = Value.Get<FVector2D>();
//
//		FVector vF = GetActorForwardVector();
//		FVector vR = GetActorRightVector();
//
//		UE_LOG(LogTemp, Warning, TEXT("Move X: %f, Y: %f"), Input.X, Input.Y);
//
//		AddMovementInput(vF, Input.X);
//		AddMovementInput(vR, Input.Y);
//	}
//}
//
//void AC_BasicPlayer::LookAction(const FInputActionValue& Value)
//{
//	if (GetController() != nullptr)
//	{
//		FVector2D Input = Value.Get<FVector2D>();
//
//		UE_LOG(LogTemp, Warning, TEXT("Look X: %f, Y: %f"), Input.X, Input.Y);
//
//		AddControllerYawInput(Input.X);
//		AddControllerPitchInput(Input.Y);
//	}
//}
//
//void AC_BasicPlayer::JumpAction()
//{
//	Super::Jump();
//}
//
//void AC_BasicPlayer::FireAction()
//{
//	// 무기 컴포넌트에서 발사 함수 호출
//}

