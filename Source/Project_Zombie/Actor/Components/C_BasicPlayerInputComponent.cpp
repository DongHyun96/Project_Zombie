#include "Actor/Components/C_BasicPlayerInputComponent.h"

#include "C_TurnInPlaceComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
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

			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// 2. Enhanced Input 컴포넌트 바인딩 (대상을 캐릭터가 아닌 'this(나 자신)'로 지정)
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IA_Move)
		{
			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::MoveStart);
			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &UC_BasicPlayerInputComponent::MoveAction);
			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Completed, this, &UC_BasicPlayerInputComponent::MoveEnd);
		}
		if (IA_Look)
		{
			EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &UC_BasicPlayerInputComponent::LookAction);
		}
		if (IA_Jump)
		{
			EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::JumpAction);
		}
		if (IA_Crouch)
		{
			EnhancedInputComponent->BindAction(IA_Crouch, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::CrouchAction);
		}
		if (IA_Fire)
		{
			EnhancedInputComponent->BindAction(IA_Fire, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::FireAction);
		}
		
		if (IA_EquipMainWeapon)
			EnhancedInputComponent->BindAction(IA_EquipMainWeapon, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::EquipMainWeapon);
		
		if (IA_EquipMeleeWeapon)
		{
			EnhancedInputComponent->BindAction(IA_EquipMeleeWeapon, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::EquipMeleeWeapon);
		}
		
		if (IA_EquipThrowable)
		{
			EnhancedInputComponent->BindAction(IA_EquipThrowable, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::EquipThrowable);
		}
		
		if (IA_ToggleArmed)
		{
			EnhancedInputComponent->BindAction(IA_ToggleArmed, ETriggerEvent::Started, this, &UC_BasicPlayerInputComponent::ToggleArmed);
		}
	}
}

void UC_BasicPlayerInputComponent::MoveStart(const FInputActionValue& Value)
{
	// TODO : Alt 키를 누른 상태의 Free Look 상황이면, MoveAction 매 Tick에서 추가 처리를 해주어야 함
	// 아래와 같이 처리를 해주면 됨
	/*PlayerMovement->bUseControllerDesiredRotation	= false;
	PlayerMovement->bOrientRotationToMovement		= false;
	Player->bUseControllerRotationYaw				= false;*/
	
	// Turn in place 동작을 하는 중이었다면, 해당 동작을 끊어준다.
	Player->GetTurnInPlaceComponent()->CancelTurnInPlaceMotionIfNecessary();

	// 일반 Movement 처리 ControllerYaw의 회전을 따라가도록 기본 처리 한다
	Player->bUseControllerRotationYaw             = true;
	PlayerMovement->bUseControllerDesiredRotation = false;
	PlayerMovement->bOrientRotationToMovement     = false;
}

void UC_BasicPlayerInputComponent::MoveAction(const FInputActionValue& Value)
{
	if (!Player->GetController()) return;

	const FVector2D Input = Value.Get<FVector2D>();
	const FVector vF      = Player->GetActorForwardVector();
	const FVector vR      = Player->GetActorRightVector();

	//UE_LOG(LogTemp, Warning, TEXT("Move X: %f, Y: %f"), Input.X, Input.Y);

	Player->AddMovementInput(vF, Input.X);
	Player->AddMovementInput(vR, Input.Y);
	
}

void UC_BasicPlayerInputComponent::MoveEnd(const FInputActionValue& Value)
{
	// Movement 멈췄으면, 다시금 TurnInPlace 처리를 할 수 있게끔 처리
	Player->GetTurnInPlaceComponent()->SetStrafeRotationToIdleStop();
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

	// Jump 이전, TurnInPlace 모션 중이었다면 중단 처리
	Player->GetTurnInPlaceComponent()->CancelTurnInPlaceMotionIfNecessary();
	
	Player->SetIsJumpInput(true);
	Player->Jump();
}

void UC_BasicPlayerInputComponent::CrouchAction()
{
	if (Player->GetPlayerMoveState() == EPlayerMoveState::Stand)
	{
		Player->StartCrouch();
	}
	else if (Player->GetPlayerMoveState() == EPlayerMoveState::Crouch)
	{
		Player->StopCrouch();
	}
}

void UC_BasicPlayerInputComponent::FireAction()
{
	// 무기 컴포넌트에서 발사 함수 호출
}

void UC_BasicPlayerInputComponent::EquipMainWeapon()
{
	UC_Util::Print("EquipMainWeapon", FColor::Red, 10.f);
}

void UC_BasicPlayerInputComponent::EquipMeleeWeapon()
{
	UC_Util::Print("EquipMeleeWeapon", FColor::Red, 10.f);
}

void UC_BasicPlayerInputComponent::EquipThrowable()
{
	UC_Util::Print("EquipThrowable", FColor::Red, 10.f);
}

void UC_BasicPlayerInputComponent::ToggleArmed()
{
	UC_Util::Print("ToggleArmed", FColor::Red, 10.f);
}

