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
#include "GameModeAndManager/C_UIManager.h"
#include "StatComponent/C_StatComponentBase.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/InvenUI/DivideWIdget/C_DivideItemWidget.h"

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

	if (!DefaultMappingContext)
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

			if (DefaultMappingContext) Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// 2. Add input actions to m_mapIA
	for (const FEnhancedActionKeyMapping& Mapping : DefaultMappingContext->GetMappings())
	{
		if (!Mapping.Action) continue;
		
		const FString ActionName = Mapping.Action->GetName();
		
		if (!m_mapIA.Contains(ActionName))
			m_mapIA.Add(ActionName, Mapping.Action);
	}
	

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
		
	
	if (const UInputAction* IA = FindIAByName(TEXT("IA_ToggleArmed")))     EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::ToggleArmed);
	if (const UInputAction* IA = FindIAByName(TEXT("IA_ToggleInventory"))) EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::ToggleInventoryWidget);
	if (const UInputAction* IA = FindIAByName(TEXT("IA_MarkPing")))        EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::MarkPing);
		
	
	if (const UInputAction* IA = FindIAByName(TEXT("IA_PlayerSprint")))
	{
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::SprintStart);
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Completed, this, &UC_BasicPlayerInputComponent::SprintEnd);	
	}
		
	if (const UInputAction* IA = FindIAByName(TEXT("IA_PlayerFire")))
	{
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::FireStarted);
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Ongoing, this, &UC_BasicPlayerInputComponent::FireOnGoing);
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Completed, this, &UC_BasicPlayerInputComponent::FireEnd);
	}
	
	
	if (const UInputAction* IA = FindIAByName(TEXT("IA_FreeLook")))
	{
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::FreeLookHolStart);
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Completed, this, &UC_BasicPlayerInputComponent::FreeLookHoldEnd);
	}
	
	if (const UInputAction* IA = FindIAByName(TEXT("IA_PlayerAim")))
	{
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::KeepAimActionStart);
		EnhancedInputComponent->BindAction(IA, ETriggerEvent::Ongoing, this, &UC_BasicPlayerInputComponent::KeepAimActionOngoing);
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
	Player->Jump();
}

void UC_BasicPlayerInputComponent::CrouchAction()
{
	if (!Player)
		return;

	Player->ToggleCrouch();
}

void UC_BasicPlayerInputComponent::FireStarted()
{
	if (AC_WeaponBase* CurWeapon = Player->GetEquippedComponent()->GetCurWeapon())
		CurWeapon->OnStartFire(Player);
}

void UC_BasicPlayerInputComponent::FireOnGoing()
{
	if (AC_WeaponBase* CurWeapon = Player->GetEquippedComponent()->GetCurWeapon())
		CurWeapon->OnFireOnGoing(Player);
}

void UC_BasicPlayerInputComponent::FireEnd()
{
	if (AC_WeaponBase* CurWeapon = Player->GetEquippedComponent()->GetCurWeapon())
		CurWeapon->OnFireEnd(Player);
}


void UC_BasicPlayerInputComponent::ReloadAction()
{
	if (AC_WeaponBase* CurWeapon = Player->GetEquippedComponent()->GetCurWeapon())
		CurWeapon->Reload(Player);
}

void UC_BasicPlayerInputComponent::KeepAimActionStart()
{
	if (!Player) return;

	AimPressStartTime = Player->GetWorld()->GetTimeSeconds();
	bIsHoldFired = false;

	Player->GetWorldTimerManager().SetTimer(
		AimHoldTimerHandle,
		this,
		&UC_BasicPlayerInputComponent::KeepAimActionOngoing,
		0.01f,
		true
	);
}

void UC_BasicPlayerInputComponent::KeepAimActionOngoing()
{
	if (!Player) return;

	float PressDuration = Player->GetWorld()->GetTimeSeconds() - AimPressStartTime;

	if (PressDuration >= HoldThreshold && !bIsHoldFired)
	{
		bIsShoulderToggled = false;

		Player->GetAimComponent()->OnAimPressed(EAimState::Shoulder);
		bIsHoldFired = true;

		Player->GetWorldTimerManager().ClearTimer(AimHoldTimerHandle);
	}
}

void UC_BasicPlayerInputComponent::KeepAimActionEnd()
{
	if (!Player) return;

	Player->GetWorldTimerManager().ClearTimer(AimHoldTimerHandle);

	if (bIsHoldFired)
	{
		Player->GetAimComponent()->OnAimReleased();
		bIsHoldFired = false;
	}
	else
	{
		if (bIsShoulderToggled)
		{
			Player->GetAimComponent()->OnAimReleased();
			bIsShoulderToggled = false;
		}
		else
		{
			Player->GetAimComponent()->OnAimPressed(EAimState::ADS);
			bIsShoulderToggled = true;
		}
	}
}

void UC_BasicPlayerInputComponent::ToggleInventoryWidget()
{
	// 1. 이 컴포넌트가 로컬 플레이어의 캐릭터에 붙어있는지 확인 (멀티플레이어 방어벽)
	if (!Player || !Player->IsLocallyControlled()) return;

	// 2. 플레이어 컨트롤러 얻어오기
	APlayerController* PC = Cast<APlayerController>(Player->GetController());
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

void UC_BasicPlayerInputComponent::EquipMainWeapon()
{
	Player->GetEquippedComponent()->Server_ChangeCurWeapon(EWeaponSlot::MainWeapon);
}

void UC_BasicPlayerInputComponent::EquipMeleeWeapon()
{
	Player->GetEquippedComponent()->Server_ChangeCurWeapon(EWeaponSlot::MeleeWeapon);
}

void UC_BasicPlayerInputComponent::EquipThrowable()
{
	Player->GetEquippedComponent()->Server_ChangeCurWeapon(EWeaponSlot::ThrowableWeapon);
}

void UC_BasicPlayerInputComponent::ToggleArmed()
{
	// TODO : 여기 지울 것 For testing (피 회복 처리)
	Player->GetStatComponent()->IncreaseCurHP(25.f);
	Player->GetEquippedComponent()->Server_ToggleArmed();
}

void UC_BasicPlayerInputComponent::FreeLookHolStart()
{
	Player->SetIsFreeLook(true);
	
	/*// TODO : 여기 지울 것 For testing
	Player->GetEquippedComponent()->Server_SetSlotWeapon(EWeaponSlot::MainWeapon, nullptr);*/
}

void UC_BasicPlayerInputComponent::FreeLookHoldEnd()
{
	Player->SetIsFreeLook(false);
}

void UC_BasicPlayerInputComponent::MarkPing()
{
	Player->GetPingSystemComponent()->TrySpawnPing();
	
	// TODO : 이 Test 코드 지우기
	Player->GetEquippedComponent()->Server_TestSpawnAllWeapons();
}

