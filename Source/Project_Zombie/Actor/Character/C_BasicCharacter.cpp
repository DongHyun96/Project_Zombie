// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Character/C_BasicCharacter.h"

#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "Engine/DamageEvents.h"
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
	// 서버 환경에서만 TakeDamage 과정을 처리할 예정
	// 클라이언트 환경인 경우, Server에게 Damage를 입은 사실을 보고 -> Server에서 실질적인 TakeDamage 처리가 들어갈 것이다
	if (!HasAuthority())
	{
		Server_TakeDamage(_DamageAmount, _DamageEvent, _EventInstigator, _DamageCauser);
		return 0.f;
	}
	
	// Damage 총량 계산
	const float DamageAmount = Super::TakeDamage(_DamageAmount, _DamageEvent, _EventInstigator, _DamageCauser);
	
	// Invalid DamageAmount early return
	if (DamageAmount <= 0.f) return 0.f;
	
	// 이미 사망한 캐릭터의 경우, 피격 처리를 하지 않는다
	if (m_StatComponent->IsCurHPZero()) return 0.f;
	
	m_StatComponent->DecreaseCurHP(DamageAmount);
	
	return DamageAmount;
}

void AC_BasicCharacter::Server_TakeDamage_Implementation
(
	float				_DamageAmount,
	FDamageEvent const& _DamageEvent,
	AController*		_EventInstigator,
	AActor*				_DamageCauser
)
{
	// 서버 쪽에서 실질적인 TakeDamage 정상 진행
	TakeDamage(_DamageAmount, _DamageEvent, _EventInstigator, _DamageCauser);
}
