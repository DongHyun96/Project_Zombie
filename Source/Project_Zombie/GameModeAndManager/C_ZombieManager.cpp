// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ZombieManager.h"

#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "Actor/Character/NPC/Enemy/Zombie/NurseZombie/C_HealingProjectile.h"
#include "Utility/C_Util.h"

UC_ZombieManager::UC_ZombieManager()
{
	// _C 없이 애셋 경로만
	static ConstructorHelpers::FClassFinder<AC_Zombie> NurseFinder(TEXT("/Game/DongHyun/Actor/Enemy/BP_NurseZombie"));

	if (NurseFinder.Succeeded())
		m_ZombieClasses.Add(EZombieType::NurseZombie, NurseFinder.Class);

	static ConstructorHelpers::FClassFinder<AC_Zombie> NormalFinder(TEXT("/Game/Harang/BP/Zombie/BP_NormalZombie"));

	if (NormalFinder.Succeeded())
		m_ZombieClasses.Add(EZombieType::NormalZombie, NormalFinder.Class);

	static ConstructorHelpers::FClassFinder<AC_Zombie> PoisonFinder(TEXT("/Game/Harang/BP/Zombie/BP_ToxicZombie"));

	if (PoisonFinder.Succeeded())
		m_ZombieClasses.Add(EZombieType::ToxicZombie, PoisonFinder.Class);
}

void UC_ZombieManager::OnWorldBeginPlay()
{
	// TODO : 실질적인 Object pooling 해두기

	// HealingProjectile Pooling 처리
	if (m_HealingProjectileClass)
	{
		for (int i = 0; i < 10; ++i)
		{
			AC_HealingProjectile* SpawnedProjectile = GetWorld()->SpawnActor<AC_HealingProjectile>(m_HealingProjectileClass);
			if (SpawnedProjectile) m_HealingProjectilePool.Add(SpawnedProjectile);
		}
	}
}

bool UC_ZombieManager::SpawnHealingProjectile
(
	const FVector& 	_SpawnLocation,
	const FVector& 	_FireDirection,
	AC_NurseZombie*	_SpawnedBy,
	AC_BasicEnemy*	_HealingTarget,
	float			_TotalHealAmount
)
{
	if (m_HealingProjectilePool.IsEmpty())
	{
		UC_Util::Print("From UC_ZombieManager::SpawnHealingProjectile : MaxPoolCnt reached!", FColor::Red, 10.f);
		return false;
	}
	
	if (!m_HealingProjectilePool.Last()->Fire(_SpawnLocation, _FireDirection, _SpawnedBy, _HealingTarget, _TotalHealAmount))
		return false; // 발사 실패
	
	// Fire 성공한 스폰된 Active한 HealingProjectile의 경우, Spawn 대기중인 Pool에서 제거
	m_HealingProjectilePool.Pop();
	return true;
}

bool UC_ZombieManager::ReturnHealingProjectileToPool(AC_HealingProjectile* _HealingProjectile)
{
	// 대상 자체가 nullptr 또는 아직 Active한 Projectile인 경우
	if (!_HealingProjectile || _HealingProjectile->IsActive())
	{
		UC_Util::Print("HealingProjectile ReturnTo Pool Failed!", FColor::MakeRandomColor(), 10.f);		
		return false;
	}

	UC_Util::Print("HealingProjectile Returning to Pool", FColor::MakeRandomColor(), 10.f);
	
	m_HealingProjectilePool.Push(_HealingProjectile);
	return true;
}
