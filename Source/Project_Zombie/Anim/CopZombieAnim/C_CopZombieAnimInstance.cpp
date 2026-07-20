// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CopZombieAnimInstance.h"

#include "Actor/Character/NPC/Enemy/Zombie/CopZombie/C_CopZombie.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "Utility/C_Util.h"

UC_CopZombieAnimInstance::UC_CopZombieAnimInstance()
{
}

void UC_CopZombieAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UC_CopZombieAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	
	if (!m_Zombie) return;
	
	m_OwnerCopZombie = Cast<AC_CopZombie>(m_Zombie);
}

void UC_CopZombieAnimInstance::NativeUpdateAnimation(float _DT)
{
	Super::NativeUpdateAnimation(_DT);

	if (!m_OwnerCopZombie) return;
	
	m_CopZombieState = m_OwnerCopZombie->GetCopZombieState();
	
	if (m_CopZombieState == ECopZombieState::WeaponEarned)
	{
		if (AC_GunBase* Gun = Cast<AC_GunBase>(m_OwnerCopZombie->GetEquippedWeapon()))
			m_LeftHandIKTranslation = Gun->GetWeaponMesh()->GetSocketLocation(TEXT("IK_Enemy_LeftHand"));
	}
}
