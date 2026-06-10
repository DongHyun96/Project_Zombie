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

#include "C_BasicPlayer.h"

AC_BasicPlayer::AC_BasicPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	
	m_SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	m_SpringArm->SetupAttachment(RootComponent);
	m_SpringArm->TargetArmLength = 250.0f;
	m_SpringArm->bUsePawnControlRotation = true;

	m_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	m_Camera->SetupAttachment(m_SpringArm);
}

void AC_BasicPlayer::BeginPlay()
{
	Super::BeginPlay();

	// 입력 시스템 초기화
	InitInput();
}

void AC_BasicPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_BasicPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* pEIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{

		if (IA_Move)
			pEIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AC_BasicPlayer::MoveAction);

		if (IA_Look)
			pEIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AC_BasicPlayer::LookAction);
		
		if (IA_Jump)
			pEIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &AC_BasicPlayer::JumpAction);
		
		if (IA_Fire)
			pEIC->BindAction(IA_Fire, ETriggerEvent::Started, this, &AC_BasicPlayer::FireAction);
	}
}

void AC_BasicPlayer::InitInput()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		return;

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
		return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	Subsystem->ClearAllMappings();

	if (Subsystem && DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}


void AC_BasicPlayer::MoveAction(const FInputActionValue& Value)
{
	if (GetController() != nullptr)
	{
		FVector2D Input = Value.Get<FVector2D>();

		FVector vF = GetActorForwardVector();
		FVector vR = GetActorRightVector();

		UE_LOG(LogTemp, Warning, TEXT("Move X: %f, Y: %f"), Input.X, Input.Y);

		AddMovementInput(vF, Input.X);
		AddMovementInput(vR, Input.Y);
	}
}

void AC_BasicPlayer::LookAction(const FInputActionValue& Value)
{
	if (GetController() != nullptr)
	{
		FVector2D Input = Value.Get<FVector2D>();

		UE_LOG(LogTemp, Warning, TEXT("Look X: %f, Y: %f"), Input.X, Input.Y);

		AddControllerYawInput(Input.X);
		AddControllerPitchInput(Input.Y);
	}
}

void AC_BasicPlayer::JumpAction()
{
	Super::Jump();
}

void AC_BasicPlayer::FireAction()
{
	// 무기 컴포넌트에서 발사 함수 호출
}
