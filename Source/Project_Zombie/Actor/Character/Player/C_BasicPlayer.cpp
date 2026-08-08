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
#include "Actor/Components/C_FeetComponent.h"

#include "GameFramework/PlayerState.h"
#include "Actor/Components/C_PlayerStatComponent.h"
#include "Controller/C_BasicPlayerController.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Net/UnrealNetwork.h"

#include "UI/InvenUI/C_InventoryGridWidget.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/InvenUI/Equipment/C_EquipmentWidget.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Utility/C_Util.h"

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

#include "TimerManager.h"
#include "Actor/Components/PlayerProfileComponent/C_PlayerProfileComponent.h"

#include "Actor/GameOverChecker/C_GameOverChecker.h"
#include "GameModeAndManager/C_GameMode_GameLv.h"

#include "GameModeAndManager/PlayerState/C_PlayerState.h"


#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "UI/InvenUI/DivideWIdget/C_DivideItemWidget.h"
#include "UI/InvenUI/Upgrade/C_ItemUpgradeWidget.h"
#include "Item/Interact/C_InteractableBase.h"
#include "Item/Interact/ItemUpgrade/C_ItemUpgradeStation.h"

#include "Kismet/GameplayStatics.h"

#include "UI/InvenUI/Upgrade/C_PlayerStatUpgradeWidget.h"

#define RECHARGED_BOOST 20.f


void AC_BasicPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
    
	// 오직 서버에서만 실행되는 안전지대
	if (!HasAuthority()) return;

	if (AC_PlayerState* PS = NewController->GetPlayerState<AC_PlayerState>())
	{
		// 1. 인벤토리 컴포넌트 복구 (저장된 데이터가 유효할 때만)
		if (PS->GetSavedInventory().Num() > 0)
		{
			if (UC_InvenComponent* InvenComp = FindComponentByClass<UC_InvenComponent>())
			{
				
				
				InvenComp->LoadInventoryFromBackup(PS->GetSavedInventory());
			}
			
			if (m_EquippedComponent && HasAuthority())
			{
				//m_EquippedComponent->SetOwnerPlayer(this);
				
				for (int32 i = 0 ; i < static_cast<int32>(EWeaponSlot::None) ; ++i)
					m_EquippedComponent->LoadEquippedWeaponFromInven(i,m_InvenComponent->GetItemAt(i));
			}
		}

		// 2. 스탯 컴포넌트 복구
		if (PS->GetSavedStats().Num() > 0)
		{
			if (UC_StatComponentBase* StatComp = FindComponentByClass<UC_StatComponentBase>())
			{
				StatComp->LoadStatsFromBackup(PS->GetSavedStats(), PS->GetSavedStatGrades());
			}
		}
        
		UE_LOG(LogTemp, Log, TEXT("[Character] PossessedBy 타이밍에 백업 데이터 정상 로드 완료."));
	}
}

AC_BasicPlayer::AC_BasicPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

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
	m_PlayerState = EPlayerState::Idle;
	m_PlayerPoseState = EPlayerPoseState::Walk;

	// 부활 변수 여부 초기화
	m_IsPendingDead = false;

	// 점프높이 설정
	GetCharacterMovement()->JumpZVelocity = 600.f;

	// 이동 속도 설정
	m_WalkSpeed = 300.f;
	m_CrouchSpeed = 200.f;
	m_BaseMaxSpeed = m_WalkSpeed; 

	//m_SprintSpeed = 600.f;	    // StatComp

	GetCharacterMovement()->MaxWalkSpeed = m_WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = m_CrouchSpeed;

	// 달리기 입력 초기화
	m_IsSprintInput = false;

	// 부스트 초기화
	//m_MaxBoost = 100.f;          // StatComp
	//m_CurBoost = m_MaxBoost;     // StatComp

	// 부스트 사용량 설정
	//m_SprintBoostUseCost = 20.f; // StatComp
	//m_BoostRecoverCost = 15.f;   // StatComp

	// 점프 입력 초기화
	m_IsJumpInput = false;

	// TeamId
	// SetGenericTeamId((uint8)ETeamType::Player); // 생성자 가상함수 호출 금지
	m_TeamId = static_cast<uint8>(ETeamType::Player);

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

	
	m_PlayerProfileComponent = CreateDefaultSubobject<UC_PlayerProfileComponent>(TEXT("PlayerProfileComponent"));

	m_InteractionComponent = CreateDefaultSubobject<UC_InteractionComponent>(TEXT("InteractionComponent"));

	m_FeetComponent = CreateDefaultSubobject<UC_FeetComponent>(TEXT("FeetComponent"));
}


void AC_BasicPlayer::Server_RequestItemUpgrade_Implementation(AC_ItemUpgradeStation* InInteractableActor, int32 InItemIndex, EUpgradableStats TargetStat)
{
	AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(GetController());
	
	if (!PC) return;
	
	if (PC->GetIsUpgradingItem()) return;
	
	PC->SetIsUpgradingItem(true);
	
	InInteractableActor->RequestItemUpgrade(this, InItemIndex, TargetStat);
}

bool AC_BasicPlayer::Server_RequestItemUpgrade_Validate(AC_ItemUpgradeStation* InInteractableActor, int32 InItemIndex, EUpgradableStats TargetStat)
{
	return true;
}

void AC_BasicPlayer::OnRep_CurBoost()
{
	if (IsLocallyControlled())
		UpdateBoostBarHUD(); 
}

void AC_BasicPlayer::BeginPlay()
{
	Super::BeginPlay();

	// InteractionComponent 활성화
	m_InteractionComponent->SetupInteraction(m_InteractionSphere);

	// GameLevelManager에 해당 Player 등록
	if (UC_GameLevelManager* LevelManager = GetWorld()->GetSubsystem<UC_GameLevelManager>())
		LevelManager->AddPlayer(this);
	


	// 웅크리기 완료 시 호출할 OnPoseTransitionFinished 바인딩
	if (m_PoseColliderHandlerComponent)
	{
		m_PoseColliderHandlerComponent
			->OnPoseTransitionFinished.AddUObject(this, &AC_BasicPlayer::OnPoseTransitionFinished);
	}

	//UpdateBoostBarHUD();

	// InventoryWidget에 Player의 InvenComponent 초기화 및 델리게이트 진행
	APlayerController* PC = Cast<APlayerController>(GetController());
	
	if (!PC) return;
	
	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	
	if (!UIManager) return;
	
	if (m_InvenComponent)
	{
		m_InvenComponent->SetHasEquipmentSlots(true);
		
		UIManager->GetInventoryWidget()->InitializeInventoryWidget();
		
		UIManager->GetInventoryWidget()->GetPlayerGridWidget()->SetInvenComponent(m_InvenComponent);
	
		UIManager->GetInventoryWidget()->GetEquipmentWidget()->InitEquipmentWidget(m_InvenComponent);
		
		UIManager->GetInventoryWidget()->GetItemUpgradeWidget()->BindingUpdateWidget(m_InvenComponent);
	}
	
	if (m_InvenComponent && m_EquippedComponent)
	{
		m_EquippedComponent->SetupInventoryComponent(m_InvenComponent);
		
	}

	if (m_StatComponent)
		UIManager->GetInventoryWidget()->GetPlayerStatUpgradeWidget()->BindStatEvents(m_StatComponent);
	
	// GameLevelManager에 해당 Player 등록
	if (UC_GameLevelManager* LevelManager = GetWorld()->GetSubsystem<UC_GameLevelManager>())
		LevelManager->AddPlayer(this);
	
	UpdateBoostBarHUD();

	//if (m_InvenComponent)
	//{
	//	UIManager->GetInventoryWidget()->GetPlayerGridWidget()->SetInvenComponent(m_InvenComponent);
	//
	//	UIManager->GetInventoryWidget()->GetEquipmentWidget()->InitEquipmentWidget(m_InvenComponent);
	//}
	

	// 플레이어의 인벤에 장비 전용 인덱스 추가.
	//m_InvenComponent->SetMaxSlots(45 + static_cast<int32>(EWeaponSlot::Max));

	// 입력 시스템 초기화
	//InitInput();
	
	UE_LOG(
	LogTemp,
	Error,
	TEXT("[PLAYER BEGINPLAY] %s / Address=%p / World=%s"),
	*GetName(),
	this,
	*GetWorld()->GetName()
);
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
	DOREPLIFETIME(AC_BasicPlayer, m_PlayerState);
	
	// 자세 상태 
	DOREPLIFETIME(AC_BasicPlayer, m_PlayerPoseState);
	
	DOREPLIFETIME(AC_BasicPlayer, m_HandState);
	DOREPLIFETIME(AC_BasicPlayer, ReplicatedAimYaw);
	DOREPLIFETIME(AC_BasicPlayer, m_CurBoost);
	DOREPLIFETIME(AC_BasicPlayer, m_bIsBoostExhausted);
}

void AC_BasicPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && m_StatComponent)
	{
		const float CurHP = m_StatComponent->GetStat(StatName::CurHP);

		if (CurHP <= 0.f && !IsDead() && !IsGettingUp() && !m_IsPendingDead)
		{
			Server_EnterDownedState();
		}
	}

	/// 나중에 스탯 컴포넌트로 분리할 예정
	// 달리기 중이면 부스트 소모
	if (HasAuthority() && !IsFalling()) // TODO : 점프중에는 실행하면 안되고 달리던 속도를 유지 혹은 점차 감소 해야 한다.
	{
		// 부스트 소모 및 회복 (서버에서만 한 번에 처리)
		const bool bIsMoving = GetVelocity().SizeSquared() > 10.f;
		const bool bIsSprinting = (m_PlayerPoseState == EPlayerPoseState::Sprint);

		if (bIsSprinting && bIsMoving && !m_bIsBoostExhausted)
		{
			ProcessSprint(DeltaTime);
		}
		else
		{
			// 이동 중이 아니거나 달리기가 아니면 부스트 회복
			RecoverBoost(m_StatComponent->GetStat(StatName::BoostRecover) * DeltaTime);
		}
	}
	
	// 달리기가 아닌 상태일 때 부스트 회복
	//if (m_PlayerPoseState != EPlayerPoseState::Sprint && m_StatComponent)
	//{
	//	RecoverBoost(m_StatComponent->GetStat(StatName::BoostRecover) * DeltaTime);
	//}

	// [Aim] 카메라 변환 중일 때만 함수 호출
	if (m_BasicPlayerAimComponent->IsTransitioningCamera())
	{
		m_BasicPlayerAimComponent->UpdateCameraInterpolation(DeltaTime);
	}

	if (IsLocallyControlled())
	{
		const FRotator ControlRot = GetControlRotation();
		const FRotator ActorRot = GetActorRotation();
		const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

		ReplicatedAimYaw = FMath::Clamp(DeltaRot.Yaw, -90.f, 90.f);

		if (HasAuthority())
		{
			RemoteViewPitch = ControlRot.Pitch * 255.f / 360.f;
		}
		else
		{
			Server_SetAimYaw(ReplicatedAimYaw);
		}
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

void AC_BasicPlayer::ToggleInventoryWidget()
{
	// 1. 이 컴포넌트가 로컬 플레이어의 캐릭터에 붙어있는지 확인 (멀티플레이어 방어벽)
	//if (!Player || !Player->IsLocallyControlled()) return;

	// 2. 플레이어 컨트롤러 얻어오기
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// 3. 컨트롤러를 통해 C_UIManager(HUD) 가져오기
	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	if (!UIManager) return;

	// 4. UIManager에서 인벤토리 위젯 가져오기
	UC_InventoryWidget* InventoryWidget = UIManager->GetInventoryWidget();
	if (!InventoryWidget) return;

	// 5. 현재 위젯의 가시성 상태에 따라 토글 처리
	if (InventoryWidget->GetVisibility() == ESlateVisibility::Visible)
	{
		if (InventoryWidget->GetDivideItemWidget()->GetVisibility() == ESlateVisibility::Visible) return;
		
		// 열려있으면 닫기
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
	else
	{
		// 닫혀있으면 열기
		InventoryWidget->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI InputMode;
		//InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
		InputMode.SetWidgetToFocus(nullptr);

		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void AC_BasicPlayer::Client_NotifyConqueringPointTower_Implementation(bool _IsCurrentlyConquering)
{
	m_PlayerInputComponent->SetPlayerIMCMode(_IsCurrentlyConquering ? EPlayerIMCMode::OnlyMovementMapping : EPlayerIMCMode::DefaultMapping);
}

void AC_BasicPlayer::Multicast_IncreaseKillCount_Implementation()
{
	++m_KillCount;
}

void AC_BasicPlayer::SetHandState(EHandState _HandState)
{
	m_HandState = _HandState;
	Server_SetHandState(_HandState);
}

UC_InteractionComponent* AC_BasicPlayer::GetInteractionComponent() const
{
	return m_InteractionComponent;
}

float AC_BasicPlayer::TakeDamage(float _Damage, FDamageEvent const& _DamageEvent, AController* _InstigatorController, AActor* _InstigatorActor)
{
	return Super::TakeDamage(_Damage, _DamageEvent, _InstigatorController, _InstigatorActor);
}

bool AC_BasicPlayer::SetCurDraggedItem(struct FInventoryEntry InEntry, UC_InvenComponent* SrcInvenComp, int32 SrcSlotIdx)
{
	// 매개변수중 하나라도 유효하지 않다면 false가 리턴됨. curDraggedItem 초기화.
	if (curDraggedItem.SetCursorItem(InEntry, SrcInvenComp, SrcSlotIdx)) return true;
	
	// TODO : 만약 false가 리턴되어도 curDraggedItem이 초기화되면 안되는 상황이 존재한다면?
	curDraggedItem.Clear();
	
	return false;
}

void AC_BasicPlayer::SetCurBoost(float NewBoost)
{
	if (HasAuthority() && m_StatComponent)
	{
		const float MaxBoost = m_StatComponent->GetStat(StatName::MaxBoost);
       
		// 0.01f보다 작은 미세 수치는 완전히 0.f로 보정
		if (NewBoost < 0.01f)
		{
			NewBoost = 0.f;
		}

		const float ClampedBoost = FMath::Clamp(NewBoost, 0.f, MaxBoost);

		m_StatComponent->SetStat(StatName::CurBoost, ClampedBoost);
		m_CurBoost = ClampedBoost; 

		OnRep_CurBoost();
	}
}

void AC_BasicPlayer::Server_SetAimYaw_Implementation(float InAimYaw)
{
	ReplicatedAimYaw = FMath::Clamp(InAimYaw, -90.f, 90.f);
}

void AC_BasicPlayer::Server_SetHandState_Implementation(EHandState _HandState)
{
	Multicast_SetHandState(_HandState);
}

bool AC_BasicPlayer::Server_SetHandState_Validate(EHandState _HandState)
{
	return true;
}

void AC_BasicPlayer::Multicast_SetHandState_Implementation(EHandState _HandState)
{
	m_HandState = _HandState;
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

bool AC_BasicPlayer::IsFalling() const
{
	if (!GetCharacterMovement())
		return false;

	return GetCharacterMovement()->IsFalling();
}

void AC_BasicPlayer::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (m_FeetComponent)
	{
		m_FeetComponent->PlayLandingSound(Hit);
	}

	if (!HasAuthority())
		return;

	if (!m_IsPendingDead)
		return;

	m_IsPendingDead = false;
	m_IsJumpInput = false;

	SetPlayerStateOnServer(EPlayerState::Dead);
}

bool AC_BasicPlayer::UseBoost(float _UseAmount)
{
	if (_UseAmount <= 0.f || !m_StatComponent) return false;

	float CurBoost = m_StatComponent->GetStat(StatName::CurBoost);
	if (CurBoost <= 0.f) return false;

	bool bHasBoost = (CurBoost >= _UseAmount);
	if (bHasBoost)
	{  
		// UI 직접 호출(UpdateBoostBarHUD)을 제거하고 SetCurBoost 사용!
		SetCurBoost(CurBoost - _UseAmount);
	}
    
	return bHasBoost;
}

void AC_BasicPlayer::RecoverBoost(float _RecoverAmount)
{
	if (!m_StatComponent) return;

	float MaxBoost = m_StatComponent->GetStat(StatName::MaxBoost);
	float CurBoost = m_StatComponent->GetStat(StatName::CurBoost);
    
	if (_RecoverAmount <= 0.f || MaxBoost <= 0.f || CurBoost >= MaxBoost)
		return;

	// 부스트 회복 계산
	float NewBoost = FMath::Min(MaxBoost, CurBoost + _RecoverAmount);

	// SetCurBoost를 사용하여 서버/클라이언트 UI 동기화 일원화!
	SetCurBoost(NewBoost);
	
	// 탈진상태 해제 조건 체크 (서버에서만 실행됨)
	if (m_bIsBoostExhausted && NewBoost >= RECHARGED_BOOST)
	{
		m_bIsBoostExhausted = false;
		OnRep_ChangedBoostExhausted();
		// TODO: 만약 방전 해제 시 UI 색상을 복구해야 한다면, RepNotify나 Delegate로 클라이언트에 전파
	}
}

void AC_BasicPlayer::StartSprint()
{
	if (!IsAlive())
		return;

	// 방전 상태이면 달리기 불가.
	if (m_bIsBoostExhausted && m_StatComponent->GetStat(StatName::CurBoost) >= RECHARGED_BOOST)
	{
		m_bIsBoostExhausted = false;
		OnRep_ChangedBoostExhausted();
	}
	
	if (m_bIsBoostExhausted) return;
	
	// 공중일 때는 달리기 불가
	if (!GetCharacterMovement() || GetCharacterMovement()->IsFalling())
		return;

	// 자세 전환 중에는 달리기 불가
	if (m_PoseColliderHandlerComponent->IsTransitioning())
		return;

	// 웅크리기 중이거나 웅크리기 전환 중일 때는 달리기 불가
	if (m_PlayerPoseState == EPlayerPoseState::Crouch)
		return;

	// 조준 상태일 때도 달리기 불가
	if (m_PlayerPoseState == EPlayerPoseState::Aim || m_BasicPlayerAimComponent->IsAiming())
	{
		return;
	}
		

	// 부스트가 없으면 달리기 불가
	if (m_StatComponent->GetStat(StatName::CurBoost) <= 0.f)
		return;

	m_IsSprintInput = true;

	if (HasAuthority())
	{
		// 서버는 서버에서 바로 PoseState를 변경
		SetPoseStateOnServer(EPlayerPoseState::Sprint);
		return;
	}
	
	// 클라이언트에서 먼저 적용
	ApplyPoseStateLocally(EPlayerPoseState::Sprint);

	Server_RequestSetPoseState(EPlayerPoseState::Sprint);
}

void AC_BasicPlayer::ProcessSprint(float DeltaTime)
{
	// 부스트 방전 상태이거나 스탯 컴포넌트가 없으면 처리 안 함
	if (m_bIsBoostExhausted || !m_StatComponent) 
	{
		StopSprint();
		return;
	}

	// 이동 중일 때만 부스트 소모 (제자리에 서서 Shift만 누르고 있을 때 소모 방지)
	if (GetVelocity().SizeSquared() > 10.f)
	{
		float Cost = m_StatComponent->GetStat(StatName::BoostCost) * DeltaTime;
		float CurBoost = m_StatComponent->GetStat(StatName::CurBoost);

		// 남아있는 부스트보다 차감량이 크면, 남은 만큼 깔끔하게 다 쓰고 0으로 만든 뒤 방전!
		if (CurBoost <= Cost)
		{
			UseBoost(CurBoost); // 남은 수치 완충 소모 (0으로 만듦)
			
			m_bIsBoostExhausted = true;
			
			OnRep_ChangedBoostExhausted();
			
			StopSprint();
		}
		else
		{
			UseBoost(Cost);
		}
	}
}

void AC_BasicPlayer::StopSprint()
{
	m_IsSprintInput = false;

	if (HasAuthority())
	{
		SetPoseStateOnServer(EPlayerPoseState::Walk);

		return;
	}
	
	// 로컬에 즉시 적용
	ApplyPoseStateLocally(EPlayerPoseState::Walk);
	
	Server_RequestSetPoseState(EPlayerPoseState::Walk);
}

void AC_BasicPlayer::ToggleCrouch()
{
	if (!IsAlive())
		return;

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

		return;
	}

	// 웅크리기 전환 가능 여부 확인
	if (NewPoseState == EPlayerPoseState::Walk)
	{
		if (!m_PoseColliderHandlerComponent->CanStand())
		{
			return;
		}
	}
	
	// 클라이언트에서 먼저 적용
	ApplyPoseStateLocally(NewPoseState);
	
	Server_RequestSetPoseState(NewPoseState);
}

void AC_BasicPlayer::ApplyPoseStateLocally(EPlayerPoseState _NewPoseState)
{
	const bool bWasCrouched = m_PlayerPoseState == EPlayerPoseState::Crouch;
	const bool bWantsToCrouch = _NewPoseState == EPlayerPoseState::Crouch;

	// 로컬에서 PoseState를 적용
	m_PlayerPoseState = _NewPoseState;

	// Crouch 동작 관련일 때 웅크리기 전환
	if (m_PoseColliderHandlerComponent && bWasCrouched != bWantsToCrouch)
	{
		m_PoseColliderHandlerComponent->StartCrouchTransition(bWantsToCrouch);
	}

	// 이동 속도 적용
	ApplyMovementSpeed();
}

/// <summary>
///	나중에 enum으로 상태를 관리하는 방식으로 변경할 듯
/// </summary>
void AC_BasicPlayer::ApplyMovementSpeed()
{
	if (!GetCharacterMovement())
		return;

	if (IsDead() || IsGettingUp())
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
		GetCharacterMovement()->MaxWalkSpeed = m_StatComponent->GetStat(StatName::SprintSpeed);
		break;
	case EPlayerPoseState::Crouch:
		GetCharacterMovement()->MaxWalkSpeed = m_CrouchSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = m_CrouchSpeed;
		break;
	case EPlayerPoseState::Aim:
		GetCharacterMovement()->MaxWalkSpeed = m_WalkSpeed;
		break;
	default:
		GetCharacterMovement()->MaxWalkSpeed = m_WalkSpeed;
		break;
	}
}

void AC_BasicPlayer::UpdateBoostBarHUD() const
{
	// 자기 자신의 Player인 경우에만 자신의 BoostBar 업데이트 처리
	//if (!IsLocallyControlled()) return;
	
	if (APlayerController* PC = GetController<APlayerController>())
	{
		if (AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD()))
		{
			if (UC_GameMainHUD* MainHUD = UIManager->GetMainHUDWidget())
				MainHUD->UpdateBoostBar(m_StatComponent->GetStat(StatName::CurBoost), m_StatComponent->GetStat(StatName::MaxBoost));
		}
	}
}


void AC_BasicPlayer::OnPoseTransitionFinished(bool _bIsCrouched)
{
	m_PlayerPoseState = _bIsCrouched ? EPlayerPoseState::Crouch : EPlayerPoseState::Walk;

	// 웅크리기 전환 완료 후 이동 속도 갱신
	ApplyMovementSpeed();
}

void AC_BasicPlayer::OnRep_ChangedBoostExhausted()
{
	if (!IsLocallyControlled())
		return;

	if (APlayerController* PC = GetController<APlayerController>())
	{
		if (AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD()))
		{
			if (UC_GameMainHUD* MainHUD = UIManager->GetMainHUDWidget())
			{
				MainHUD->ChangeBoostBarColor(m_bIsBoostExhausted);
			}
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

void AC_BasicPlayer::SetPlayerStateOnServer(EPlayerState _NewState)
{
	if (!HasAuthority())
		return;

	if (m_PlayerState == _NewState)
		return;

	m_PlayerState = _NewState;

	ApplyPlayerState();

	ForceNetUpdate();
}

void AC_BasicPlayer::ActivateInteractionUI(const FText& _InteractionText)
{
	if (!IsLocallyControlled())
		return;

	if (APlayerController* PC = GetController<APlayerController>())
	{
		if (AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD()))
		{
			if (UC_GameMainHUD* MainHUD = UIManager->GetMainHUDWidget())
			{
				MainHUD->ActivateInteractionUI(_InteractionText);
			}
		}
	}
}

void AC_BasicPlayer::DeactivateInteractionUI()
{
	if (!IsLocallyControlled())
		return;

	if (APlayerController* PC = GetController<APlayerController>())
	{
		if (AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD()))
		{
			if (UC_GameMainHUD* MainHUD = UIManager->GetMainHUDWidget())
			{
				MainHUD->DeactivateInteractionUI();
			}
		}
	}
}

void AC_BasicPlayer::ActivateInteractionTimerUI(float _Duration)
{
	if (!IsLocallyControlled())
		return;

	if (APlayerController* PC = GetController<APlayerController>())
	{
		if (AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD()))
		{
			if (UC_GameMainHUD* MainHUD = UIManager->GetMainHUDWidget())
			{
				MainHUD->ActivateInteractionTimer(_Duration);
			}
		}
	}
}

void AC_BasicPlayer::DeactivateInteractionTimerUI()
{
	if (!IsLocallyControlled())
		return;

	if (APlayerController* PC = GetController<APlayerController>())
	{
		if (AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD()))
		{
			if (UC_GameMainHUD* MainHUD = UIManager->GetMainHUDWidget())
			{
				MainHUD->DeactivateInteractionTimer();
			}
		}
	}
}

void AC_BasicPlayer::Server_EnterDownedState_Implementation()
{
	if (!HasAuthority())
		return;

	// 이미 사망 상태라면 처리하지 않음
	//if (!IsAlive())
	//	return;

	// 체력이 남아 있으면 사망 처리하지 않음
	if (m_StatComponent->GetStat(StatName::CurHP) > 0.f)
		return;

	// 사망 관련 상태 중복 처리 방지
	if (IsDead() || IsGettingUp() || m_IsPendingDead)
		return;

	m_IsSprintInput = false;
	m_IsJumpInput = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(m_GetUpTimerHandle);
	}

	// 공중에서 죽었을 경우 Dead 를 보류
	if (GetCharacterMovement()->IsFalling())
	{
		m_IsPendingDead = true;
		return;
	}

	SetPlayerStateOnServer(EPlayerState::Dead); 
	
	// 게임 오버 상황인지 체크 (모든 플레이어들이 그로기 상태로 접어든 상황인 경우)
	if (LEVEL_MANAGER->HasAllPlayerDead())
	{
		// 게임 오버 상황
		GAME_LV_GAME_MODE(GetWorld())->GetGameOverChecker()->Multicast_GameOver(false);
	}
}

void AC_BasicPlayer::StartGettingUp()
{
	// 서버에서만 처리
	if (!HasAuthority())
		return;

	if (!IsDead())
		return;

	// 타이머 중복 방지
	SetPlayerStateOnServer(EPlayerState::GettingUp);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(m_GetUpTimerHandle);

		World->GetTimerManager().SetTimer(
			m_GetUpTimerHandle,
			this,
			&AC_BasicPlayer::FinishGettingUp,
			1.4f, // TODO: 하드코딩된 시간, 나중에 몽타주 길이에 맞춰서 변경 필요 // GettingUp 시퀀스 길이와 비슷하게 설정
			false
		);
	}
}

void AC_BasicPlayer::FinishGettingUp()
{
	if (!HasAuthority())
		return;

	if (!IsGettingUp())
		return;
	
	// TODO: 몽타주 재생 등 추가적인 처리를 여기에 

	m_StatComponent->SetCurHP(10.0f);


	SetPlayerStateOnServer(EPlayerState::Idle);
}

void AC_BasicPlayer::ApplyPlayerState()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
		return;

	switch (m_PlayerState)
	{
	case EPlayerState::Idle:
	{
		if (MovementComponent->MovementMode == MOVE_None)
		{
			MovementComponent->SetMovementMode(MOVE_Walking);
		}

		// 현재 자세에 맞는 이동 속도 적용
		ApplyMovementSpeed();
		break;
	}
	case EPlayerState::Reviving:
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
		break;
	}
	case EPlayerState::Dead:
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
		break;
	}
	case EPlayerState::GettingUp:
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
		break;
	}
	}
}

void AC_BasicPlayer::OnRep_PlayerState()
{
	ApplyPlayerState();
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

	const bool bWasCrouching = m_PlayerPoseState == EPlayerPoseState::Crouch;
	const bool bWantsToCrouch = _NewPoseState == EPlayerPoseState::Crouch;

	// Crouch 상태 전환 시에만 호출
	if (bWasCrouching != bWantsToCrouch)
	{
		bool bStarted = m_PoseColliderHandlerComponent->SetCrouched(bWantsToCrouch);

		if (!bStarted)
			return;
	}

	m_PlayerPoseState = _NewPoseState;

	// 자세 전환 중에 잠깐 멈춤
	ApplyMovementSpeed();

	// 상태를 가능한 빨리 변경
	ForceNetUpdate();
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
	APlayerController* pPC = Cast<APlayerController>(GetController());
	
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
	
	TempEntry.CurCount = ActualDropCount;

	// 2. 안전하게 깎인 게 확인되었으므로, 서버 월드에 실제 아이템 액터 스폰
	UC_ItemManager* ItemManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	if (ItemManager) ItemManager->DropItemByPlayer(TempEntry, this);
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

