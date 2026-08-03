// Fill out your copyright notice in the Description page of Project Settings.

#include "C_ZombieAnimInstance.h"

#include "../Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"

#include "Actor/Character/NPC/Enemy/Zombie/TankZombie/C_TankZombie.h"
#include "GameModeAndManager/C_UIManager.h"

#include "Utility/C_Util.h"
//
void UC_ZombieAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UC_ZombieAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	m_Zombie            = Cast<AC_Zombie>(TryGetPawnOwner());
	m_MovementComponent = m_Zombie->GetCharacterMovement();
}

void UC_ZombieAnimInstance::NativeUpdateAnimation(float _DT)
{
	Super::NativeUpdateAnimation(_DT);

	if (nullptr == m_Zombie || nullptr == m_MovementComponent)
		return;

	const FVector Velocity = m_Zombie->GetVelocity();

	m_GroundSpeed = Velocity.Size2D();
	
	if (10.f < m_GroundSpeed)
		m_Direction = CalculateDirection(Velocity, m_Zombie->GetActorRotation());

	m_VerticalSpeed = m_MovementComponent->Velocity.Z;
}

void UC_ZombieAnimInstance::AnimNotify_SkillEnd()
{
	//UC_Util::Print("AnimNotify_SkillEnd");
	if (!IsValid(m_Zombie)) return;

	// 서버 쪽에서만 Skill 동작의 EndSkill 처리 담당 (Skill 끝 처리 등의 주체는 서버)
	if (!m_Zombie->IsLocallyControlled()) return;	

	if (AC_TankZombie* TankZombie = Cast<AC_TankZombie>(m_Zombie))
	{
		TankZombie->FinishChargeSkill();
	}

	if (UC_EnemySkillComponent* SkillCom = m_Zombie->GetSkillComponent())
	{
		SkillCom->OnAN_EndSkill();
	}
}

void UC_ZombieAnimInstance::AnimNotify_Fire()
{
	if(m_Zombie && m_Zombie->IsLocallyControlled())
	{
		m_Zombie->GetSkillComponent()->Fire();
	}
}

void UC_ZombieAnimInstance::AnimNotify_ChargeStart()
{
	AC_TankZombie* Tank = Cast<AC_TankZombie>(TryGetPawnOwner());

	if (!IsValid(Tank) || !Tank->IsLocallyControlled()) return;

	Tank->BeginPreparedCharge();
}

void UC_ZombieAnimInstance::AnimNotify_LandingImpact()
{
	AC_TankZombie* Tank = Cast<AC_TankZombie>(TryGetPawnOwner());

	if (!IsValid(Tank))
		return;

	Tank->LandingImpact();
}