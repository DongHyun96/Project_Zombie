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
#include "GameMode/C_UIManager.h"
#include "UI/InvenUI/C_InventoryGridWidget.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/MainHUD/C_GameMainHUD.h"

AC_BasicPlayer::AC_BasicPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	
	m_SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	m_SpringArm->SetupAttachment(RootComponent);
	m_SpringArm->TargetArmLength = 250.0f;
	m_SpringArm->bUsePawnControlRotation = true;

	m_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	m_Camera->SetupAttachment(m_SpringArm);

	// 캐릭터 상태 초기화
	m_PlayerMoveSpeedState = EPlayerMoveSpeedState::Walk;

	// 점프높이 설정
	GetCharacterMovement()->JumpZVelocity = 600.f;

	// 이동 속도 설정
	m_WalkSpeed = 300.f;
	m_SprintSpeed = 600.f;
	m_CrouchSpeed = 200.f;

	m_BaseMaxSpeed = m_WalkSpeed;

	m_IsCrouchTransitioning = false;

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

	// 웅크리기 전환 시간 설정
	m_CrouchTransitionStopTime = 0.2f;

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
}

void AC_BasicPlayer::BeginPlay()
{
	Super::BeginPlay();

	UpdateBoostBarHUD();

	// InventoryWidget에 Player의 InvenComponent 초기화 및 델리게이트 진행
	APlayerController* PC = Cast<APlayerController>(GetController());
	
	if (!PC) return;
	
	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	
	if (!UIManager) return;
	
	UIManager->GetInventoryWidget()->GetPlayerGridWidget()->SetInvenComponent(m_InvenComponent);
	// 까지
	
	
	// 입력 시스템 초기화
	//InitInput();
}

void AC_BasicPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/// 나중에 스탯 컴포넌트로 분리할 예정
	// 달리기 중이면 부스트 소모
	if (m_PlayerMoveSpeedState == EPlayerMoveSpeedState::Sprint)
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
	return 0.0f;
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
	m_IsSprintInput = true;

	// 공중일 때는 달리기 불가
	if (!GetCharacterMovement() || GetCharacterMovement()->IsFalling())
		return;

	// 웅크리기 중이거나 웅크리기 전환 중일 때는 달리기 불가
	if (m_PlayerMoveSpeedState == EPlayerMoveSpeedState::Crouch || m_IsCrouchTransitioning)
		return;

	// 부스트가 없으면 달리기 불가
	if (m_CurBoost <= 0.f)
		return;

	m_PlayerMoveSpeedState = EPlayerMoveSpeedState::Sprint;

	ApplyMovementSpeed();
}

void AC_BasicPlayer::StopSprint()
{
	m_IsSprintInput = false;

	m_PlayerMoveSpeedState = EPlayerMoveSpeedState::Walk;

	ApplyMovementSpeed();
}

void AC_BasicPlayer::ToggleCrouch()
{
	if (m_IsCrouchTransitioning)
		return;

	if (GetCharacterMovement()->IsFalling())
		return;

	m_IsCrouchTransitioning = true;

	// 전환 시작 순간 잠깐 정지
	GetCharacterMovement()->MaxWalkSpeed = 0.f;

	if (m_PlayerMoveSpeedState != EPlayerMoveSpeedState::Crouch)
	{
		// 웅크리기 시작 시 달리기 입력 해제
		m_IsSprintInput = false;

		m_PlayerMoveSpeedState = EPlayerMoveSpeedState::Crouch;

		Crouch();

		GetWorldTimerManager().SetTimer(
			m_CrouchTransitionTimerHandle,
			this,
			&AC_BasicPlayer::ApplyCrouchSpeed,
			m_CrouchTransitionStopTime,
			false
		);
	}
	else
	{
		m_PlayerMoveSpeedState = EPlayerMoveSpeedState::Walk;

		UnCrouch();

		GetWorldTimerManager().SetTimer(
			m_CrouchTransitionTimerHandle,
			this,
			&AC_BasicPlayer::ApplyWalkSpeed,
			m_CrouchTransitionStopTime,
			false
		);
	}
}

/// <summary>
///	나중에 enum으로 상태를 관리하는 방식으로 변경할 듯
/// </summary>
void AC_BasicPlayer::ApplyMovementSpeed()
{
	if (!GetCharacterMovement())
		return;

	// 웅크리기 전환 중일 때는 잠깐 정지
	if (m_IsCrouchTransitioning)
	{
		GetCharacterMovement()->MaxWalkSpeed = 0.f;
		return;
	}

	// 상태에 따른 이동 속도 적용
	switch (m_PlayerMoveSpeedState)
	{
	case EPlayerMoveSpeedState::Walk:
		GetCharacterMovement()->MaxWalkSpeed = m_WalkSpeed;
		break;
	case EPlayerMoveSpeedState::Sprint:
		GetCharacterMovement()->MaxWalkSpeed = m_SprintSpeed;
		break;
	case EPlayerMoveSpeedState::Crouch:
		GetCharacterMovement()->MaxWalkSpeed = m_CrouchSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = m_CrouchSpeed;
		break;
	case EPlayerMoveSpeedState::Aim:
		//GetCharacterMovement()->MaxWalkSpeed = m_AimSpeed;
		break;
	default:
		GetCharacterMovement()->MaxWalkSpeed = m_WalkSpeed;
		break;
	}
}

void AC_BasicPlayer::ApplyCrouchSpeed()
{
	m_IsCrouchTransitioning = false;

	ApplyMovementSpeed();
}

void AC_BasicPlayer::ApplyWalkSpeed()
{
	m_IsCrouchTransitioning = false;

	ApplyMovementSpeed();
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

bool AC_BasicPlayer::Server_RequestMoveItem_Validate(UC_InvenComponent* SrcComp, int32 SrcIdx,
	UC_InvenComponent* DstComp, int32 DstIdx)
{
	return (SrcComp != nullptr && DstComp != nullptr);
}

void AC_BasicPlayer::Server_RequestMoveItem_Implementation(UC_InvenComponent* SrcComp, int32 SrcIdx,
	UC_InvenComponent* DstComp, int32 DstIdx)
{
	if (SrcComp == DstComp)
	{
		// 동일 인벤토리 내부 스왑인 경우
		SrcComp->SwapInvenEntry(SrcIdx, DstIdx);
	}
	else
	{
		// 플레이어 가방 <-> 창고 컴포넌트 간 이동인 경우
		SrcComp->TransferItemTo(SrcIdx, DstComp, DstIdx);
	}
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

