#include "Actor/Components/C_BasicPlayerInputComponent.h"

#include "C_EquippedComponent.h"
#include "C_PingSystemComponent.h"
#include "C_TurnInPlaceComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "C_BasicPlayerAimComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "Actor/Components/InteractionComponent/C_InteractionComponent.h"
#include "GameModeAndManager/C_UIManager.h"
#include "StatComponent/C_StatComponentBase.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/InvenUI/DivideWIdget/C_DivideItemWidget.h"
#include "UI/MainHUD/C_GameMainHUD.h"

#include "Utility/C_Util.h"

UC_BasicPlayerInputComponent::UC_BasicPlayerInputComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UC_BasicPlayerInputComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


void UC_BasicPlayerInputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UC_BasicPlayerInputComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent, AC_BasicPlayer* InPlayer)
{
	Player = InPlayer;
	if (!Player) return;

	if (!DefaultMappingContext || !OnlyMovementMappingContext)
	{
		UC_Util::Print("UC_BasicPlayerInputComponent::InitializePlayerInput : Init DefaultMappingContext on BPC_Player InputComponent", FColor::Red, 10.f);
		return;
	}
	
	// 캐릭터의 무브먼트 컴포넌트 주소 확보
	PlayerMovement = Player->GetCharacterMovement();

	// 1. Mapping Context 등록 (이전 프로젝트의 SetPlayerMappingContext 로직 통합)
	if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// 기존에 등록된 MappingContext 제거 후 새로 등록
			// Look 쪽 매핑이 등록되어 있으면 기존 매핑을 제거하고 새로 등록
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// 2. Add input actions to m_mapIA
	auto PopulateIAMap = [this](const UInputMappingContext* Context) 
	{
		if (!Context) return;
		for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings()) 
		{
			if (Mapping.Action && !m_mapIA.Contains(Mapping.Action->GetName())) 
				m_mapIA.Add(Mapping.Action->GetName(), Mapping.Action);
		}
	};

	PopulateIAMap(DefaultMappingContext);
	PopulateIAMap(OnlyMovementMappingContext);
	

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UC_Util::Print("UC_BasicPlayerInputComponent::InitializePlayerInput : Plead use EnhancedInputComponent", FColor::Red, 10.f);
		return;
	}
	
	// 3. Enhanced Input 컴포넌트 바인딩 (대상을 캐릭터가 아닌 'this(나 자신)'로 지정)
	if (const UInputAction* IA = FindIAByName(TEXT("IA_PlayerMove")))   EnhancedInputComponent->BindAction(IA, ETriggerEvent::Triggered, this, &UC_BasicPlayerInputComponent::MoveAction);
	if (const UInputAction* IA = FindIAByName(TEXT("IA_PlayerLook")))   EnhancedInputComponent->BindAction(IA, ETriggerEvent::Triggered, this, &UC_BasicPlayerInputComponent::LookAction);
	if (const UInputAction* IA = FindIAByName(TEXT("IA_PlayerJump")))   EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::JumpAction);
	if (const UInputAction* IA = FindIAByName(TEXT("IA_PlayerCrouch"))) EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::CrouchAction);
	if (const UInputAction* IA = FindIAByName(TEXT("IA_PlayerReload"))) EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::ReloadAction);

	if (const UInputAction* IA = FindIAByName(TEXT("IA_EquipMainWeapon")))  EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::EquipMainWeapon);
	if (const UInputAction* IA = FindIAByName(TEXT("IA_EquipMeleeWeapon"))) EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::EquipMeleeWeapon);
	if (const UInputAction* IA = FindIAByName(TEXT("IA_EquipThrowable")))   EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::EquipThrowable);
	if (const UInputAction* IA = FindIAByName(TEXT("IA_EquipPotion")))		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::EquipPotion);
		
	if (const UInputAction* IA = FindIAByName(TEXT("IA_PlayerInteract")))     EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::InteractionAction);
	
	if (const UInputAction* IA = FindIAByName(TEXT("IA_ToggleArmed")))     EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::ToggleArmed);
	if (const UInputAction* IA = FindIAByName(TEXT("IA_ToggleInventory"))) EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::ToggleInventoryWidget);
	if (const UInputAction* IA = FindIAByName(TEXT("IA_ToggleMenu")))	   EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::ToggleMenuWidget);
	if (const UInputAction* IA = FindIAByName(TEXT("IA_MarkPing")))        EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::MarkPing);
		
	
	if (const UInputAction* IA = FindIAByName(TEXT("IA_PlayerSprint")))
	{
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::SprintStart);
		
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Completed, this, &UC_BasicPlayerInputComponent::SprintEnd);
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Canceled, this, &UC_BasicPlayerInputComponent::SprintEnd);
	}
		
	if (const UInputAction* IA = FindIAByName(TEXT("IA_PlayerFire")))
	{
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::FireStarted);
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Ongoing, this, &UC_BasicPlayerInputComponent::FireOnGoing);
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Completed, this, &UC_BasicPlayerInputComponent::FireEnd);
	}
	
	if (const UInputAction* IA = FindIAByName(TEXT("IA_FireMode")))
	{
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::SwitchFireModeAction);
	}
	
	if (const UInputAction* IA = FindIAByName(TEXT("IA_FreeLook")))
	{
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Triggered, this, &UC_BasicPlayerInputComponent::FreeLooking);
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Completed, this, &UC_BasicPlayerInputComponent::FreeLookHoldEnd);
	}
	
	if (const UInputAction* IA = FindIAByName(TEXT("IA_PlayerAim")))
	{
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::KeepAimActionStart);
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Completed, this, &UC_BasicPlayerInputComponent::KeepAimActionEnd);
	}

}

const UInputAction* UC_BasicPlayerInputComponent::FindIAByName(const FString& _Name)
{
	const UInputAction** pAction = m_mapIA.Find(_Name);

	if (!pAction)
	{
		UC_Util::Print("From UC_BasicPlayerInputComponent::FindIAByName : " +  _Name + " -> Cannot find corresponding IA name", FColor::Red, 10.f);
		return nullptr;
	}
	
	// return !pAction ? nullptr : *pAction;
	return *pAction;
}

void UC_BasicPlayerInputComponent::SetPlayerIMCMode(EPlayerIMCMode _IMCMode)
{
	if (!Player) return;

	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (!PC) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem) return;

	if (_IMCMode == EPlayerIMCMode::OnlyMovementMapping)
	{
		if (UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld()))
		MainHUD->AddPlayerWarningLog("ONLY MOVEMENT IS ALLOWED");
		
		// 1. 사격, 조준 등 누르고 있던 행동 상태를 명시적으로 끝냄
		FireEnd();
		KeepAimActionEnd();

		// 2. DefaultMappingContext 제거 및 OnlyMovementMappingContext 적용
		if (DefaultMappingContext) Subsystem->RemoveMappingContext(DefaultMappingContext);
		if (OnlyMovementMappingContext) Subsystem->AddMappingContext(OnlyMovementMappingContext, 0);
	}
	else
	{
		// 1. OnlyMovementMappingContext 제거 및 DefaultMappingContext 복구
		if (OnlyMovementMappingContext) Subsystem->RemoveMappingContext(OnlyMovementMappingContext);
		if (DefaultMappingContext) Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void UC_BasicPlayerInputComponent::MoveAction(const FInputActionValue& Value)
{
	if (!Player->GetController()) return;

	// 웅크리기 전환 중이라면, 이동 입력을 무시
	if (Player->IsCrouchTransitioning())
		return;

	const FVector2D Input = Value.Get<FVector2D>();
	const FVector vF      = Player->GetActorForwardVector();
	const FVector vR      = Player->GetActorRightVector();

	//UE_LOG(LogTemp, Warning, TEXT("Move X: %f, Y: %f"), Input.X, Input.Y);

	Player->AddMovementInput(vF, Input.X);
	Player->AddMovementInput(vR, Input.Y);
	
}

void UC_BasicPlayerInputComponent::SprintStart()
{
	if (!Player)
		return;

	Player->StartSprint();
}

void UC_BasicPlayerInputComponent::SprintEnd()
{
	if (!Player)
		return;

	Player->StopSprint();
}

void UC_BasicPlayerInputComponent::LookAction(const FInputActionValue& Value)
{
	if (Player->GetController() != nullptr)
	{
		FVector2D Input = Value.Get<FVector2D>();

		//UE_LOG(LogTemp, Warning, TEXT("Look X: %f, Y: %f"), Input.X, Input.Y);

		Player->AddControllerYawInput(Input.X);
		Player->AddControllerPitchInput(Input.Y);
	}
}

void UC_BasicPlayerInputComponent::JumpAction()
{
	if (!Player || !Player->CanJump()) return;

	// Crouch 상태라면, 먼저 Crouch를 풀어주도록 처리
	if (Player->IsCrouching())
	{
		Player->ToggleCrouch();
		return;
	}

	Player->SetIsJumpInput(true);
	
	if (Player->IsSprinting()) 
		Player->StopSprint();
	
	Player->Jump();
}

void UC_BasicPlayerInputComponent::CrouchAction()
{
	if (!Player)
		return;

	Player->ToggleCrouch();
}

void UC_BasicPlayerInputComponent::InteractionAction()
{
	if (!Player || !Player->IsLocallyControlled())
		return;

	UC_InteractionComponent* InteractionComponent = Player->GetInteractionComponent();
	if (!InteractionComponent)
		return;

	InteractionComponent->TryInteract();
}

void UC_BasicPlayerInputComponent::FireStarted()
{
	if (AC_WeaponBase* CurWeapon = Player->GetEquippedComponent()->GetCurWeapon())
	{
		CurWeapon->OnStartFire(Player);
		Player->SetIsFiring(true);
		if (Player->IsFreeLook())
		{
			Player->SetIsFreeLook(false); // 사격 시작 시, FreeLook 기능이 만약 활성화 중이었다면 해당 기능 비활성화 처리
			
			if (UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld()))
				MainHUD->AddPlayerWarningLog("FREE LOOK RELEASED");
		}
	}
}

void UC_BasicPlayerInputComponent::FireOnGoing()
{
	if (AC_WeaponBase* CurWeapon = Player->GetEquippedComponent()->GetCurWeapon())
		CurWeapon->OnFireOnGoing(Player);
}

void UC_BasicPlayerInputComponent::FireEnd()
{
	if (AC_WeaponBase* CurWeapon = Player->GetEquippedComponent()->GetCurWeapon())
	{
		CurWeapon->OnFireEnd(Player);
		Player->SetIsFiring(false);
	}
}


void UC_BasicPlayerInputComponent::ReloadAction()
{
	if (AC_WeaponBase* CurWeapon = Player->GetEquippedComponent()->GetCurWeapon())
		CurWeapon->Reload(Player);
}

void UC_BasicPlayerInputComponent::SwitchFireModeAction()
{
	if (AC_WeaponBase* CurWeapon = Player->GetEquippedComponent()->GetCurWeapon())
		CurWeapon->SwitchFireMode();
}

void UC_BasicPlayerInputComponent::KeepAimActionStart()
{
	if (!Player) return;

	Player->GetAimComponent()->OnAimPressed();
}

void UC_BasicPlayerInputComponent::KeepAimActionEnd()
{
	if (!Player) return;

	Player->GetAimComponent()->OnAimReleased();
}

void UC_BasicPlayerInputComponent::ToggleInventoryWidget()
{
	Player->ToggleInventoryWidget();
}

void UC_BasicPlayerInputComponent::ToggleMenuWidget()
{
	Player->ToggleMenuWidget();
}

void UC_BasicPlayerInputComponent::EquipMainWeapon()
{
	Player->GetEquippedComponent()->ChangeCurWeapon(EWeaponSlot::MainWeapon);
}

void UC_BasicPlayerInputComponent::EquipMeleeWeapon()
{
	Player->GetEquippedComponent()->ChangeCurWeapon(EWeaponSlot::MeleeWeapon);
}

void UC_BasicPlayerInputComponent::EquipThrowable()
{
	Player->GetEquippedComponent()->ChangeCurWeapon(EWeaponSlot::ThrowableWeapon);
}

void UC_BasicPlayerInputComponent::EquipPotion()
{
	Player->GetEquippedComponent()->ChangeCurWeapon(EWeaponSlot::Potion);
}

void UC_BasicPlayerInputComponent::ToggleArmed()
{
	// TODO : 여기 지울 것 For testing (피 회복 처리)
	/*Player->GetStatComponent()->IncreaseCurHP(25.f);*/
	Player->GetEquippedComponent()->ToggleArmed();
	
	/*if (Player->GetEquippedComponent()->GetCurWeapon())
		Player->GetEquippedComponent()->GetCurWeapon()->OnSheathStart();*/
}

void UC_BasicPlayerInputComponent::FreeLookHoldStart()
{
	// 이미 무기의 사격(LMB)를 진행중이라면 Warning Log 띄우기 (이 Start trigger는 오로지 UI Log 한번 띄우기용)
	if (Player->IsFiring())
	{
		if (UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld()))
			MainHUD->AddPlayerWarningLog("CANNOT USE FREE LOOK WHILE USING WEAPON");
	}
}

void UC_BasicPlayerInputComponent::FreeLooking()
{
	// 이미 무기의 사격(LMB)를 진행중이라면 return
	if (Player->IsFiring()) return;
	
	Player->SetIsFreeLook(true);
	Player->SetAltFlag(false);

	// AimDown을 하던 중이었던 아니든, Gun을 들고 있는 경우, Aim 풀어줌
	if (Cast<AC_GunBase>(Player->GetEquippedComponent()->GetCurWeapon()))
		Player->GetAimComponent()->OnAimReleased();

	if (Player->GetController())
	{
		const FRotator PlayerActorRotation = Player->GetActorRotation();
		FRotator CharacterMovingDirection{};
		CharacterMovingDirection.Yaw   = PlayerActorRotation.Yaw;
		CharacterMovingDirection.Pitch = PlayerActorRotation.Pitch;
		CharacterMovingDirection.Roll  = Player->GetController()->GetControlRotation().Roll;
		
		Player->SetPlayerMovingDirection(CharacterMovingDirection);
	}
}

void UC_BasicPlayerInputComponent::FreeLookHoldEnd()
{
	Player->SetIsFreeLook(false);
	Player->SetAltFlag(true);
}

void UC_BasicPlayerInputComponent::MarkPing()
{
	Player->GetPingSystemComponent()->TrySpawnPing();
}

