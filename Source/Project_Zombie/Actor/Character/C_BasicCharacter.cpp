// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Character/C_BasicCharacter.h"

#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "GameFramework/CharacterMovementComponent.h"

AC_BasicCharacter::AC_BasicCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	
}

void AC_BasicCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->bEnablePhysicsInteraction = false;
}

void AC_BasicCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AC_BasicCharacter::TakeDamage
(
	float				_DamageAmount,
	FDamageEvent const& _DamageEvent,
	AController*		_EventInstigator,
	AActor*				_DamageCauser
)
{
	// Damage 총량 계산
	const float DamageAmount = Super::TakeDamage(_DamageAmount, _DamageEvent, _EventInstigator, _DamageCauser);
	
	// Invalid DamageAmount early return
	if (DamageAmount <= 0.f) return 0.f;
	
	// 서버 환경에서만 TakeDamage 과정을 처리할 예정
	if (!HasAuthority()) return 0.f;

	m_StatComponent->DecreaseCurHP(DamageAmount);
	
	return DamageAmount;
}
