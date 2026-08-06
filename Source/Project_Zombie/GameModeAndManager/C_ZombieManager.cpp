// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ZombieManager.h"

#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "Actor/Character/NPC/Enemy/Zombie/NurseZombie/C_NurseZombie.h"
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

	static ConstructorHelpers::FClassFinder<AC_Zombie> TankFinder(TEXT("/Game/Harang/BP/Zombie/BP_TankZombie"));

	if (TankFinder.Succeeded())
		m_ZombieClasses.Add(EZombieType::TankZombie, TankFinder.Class);
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

bool UC_ZombieManager::ReturnZombieToPool(AC_Zombie* _Zombie)
{
	// 반환할 Zombie가 유효하지 않은 경우 실패
	if (!IsValid(_Zombie))
		return false;

	// 매니저는 서버에서만 관리
	UWorld* World = GetWorld();
	if (!IsValid(World) || World->GetNetMode() == NM_Client)
		return false;

	const EZombieType ZombieType = _Zombie->GetZombieType();

	// 해당 좀비타입의 대기 풀을 가져오거나 없으면 생성
	TArray<AC_Zombie*>& pPool = m_ZombiePool.FindOrAdd(ZombieType);

	// 동일 좀비 풀 중복 등록 방지
	if (pPool.Contains(_Zombie))
	{
		UC_Util::Print("Zombie already exists in pool", FColor::Red, 10.f);
		return false;
	}

	// 좀비 자신을 풀 대기상태로 전환
	// 표시/충돌/Tick 상태 비활성화
	if (!_Zombie->DeactivateForPool())
	{
		UC_Util::Print("DeactivateForPool Failed !!", FColor::Red, 10.f);

		return false;
	}

	// 좀비 활성 목록에서 제거
	if (TSet<AC_Zombie*>* ActiveSet = m_ActiveZombies.Find(ZombieType))
	{
		ActiveSet->Remove(_Zombie);
	}

	// NurseZombie는 전용 활성목록에서도 제거
	if (ZombieType == EZombieType::NurseZombie)
	{
		if (AC_NurseZombie* NurseZombie = Cast<AC_NurseZombie>(_Zombie))
		{
			m_ActiveNurseZombies.Remove(NurseZombie);
		}
	}

	// 타입별 풀에 다시 대기 등록
	pPool.Add(_Zombie);

	return true;
}

AC_Zombie* UC_ZombieManager::SpawnZombieFromPool(EZombieType _ZombieType, const FTransform& _SpawnTransform)
{
	UWorld* World = GetWorld();

	// 풀 관리는 서버에서만 
	if (!IsValid(World) || World->GetNetMode() == NM_Client)
		return nullptr;

	// 지정된 타입의 풀 검색
	TArray<AC_Zombie*>* pPool = m_ZombiePool.Find(_ZombieType);

	if (!pPool || pPool->IsEmpty())
	{
		UC_Util::Print("From UC_ZombieManager::SpawnZombieFromPool : Pool is empty!!", FColor::Red, 10.f);
		return nullptr;
	}

	// 풀의 마지막 좀비를 확인
	AC_Zombie* Zombie = pPool->Last();

	if (!IsValid(Zombie))
	{
		// 잘못된 포인터는 풀에서 제거
		pPool->Pop();

		return nullptr;
	}

	// 활성화 성공 후에만 풀에서 제거
	if (!Zombie->ActivateFromPool(_SpawnTransform))
	{
		return nullptr;
	}

	// 대기 풀에서 제거
	pPool->Pop();

	// 필드 활성 목록에 등록
	m_ActiveZombies.FindOrAdd(_ZombieType).Add(Zombie);

	// NurseZombie 라면 전용 활성 목록에도 등록
	if (_ZombieType == EZombieType::NurseZombie)
	{
		if (AC_NurseZombie* NurseZombie = Cast<AC_NurseZombie>(Zombie))
		{
			m_ActiveNurseZombies.Add(NurseZombie);
		}
	}

	return Zombie;
}

bool UC_ZombieManager::TestSpawnNormalZombie()
{
	UWorld* World = GetWorld();

	// 서버에서만 테스트 스폰 처리
	if (!IsValid(World) ||
		World->GetNetMode() == NM_Client)
	{
		return false;
	}

	// 임시 고정 스폰 위치
	const FVector SpawnLocation(
		0.f,
		0.f,
		300.f);

	const FRotator SpawnRotation(
		0.f,
		180.f,
		0.f);

	const FTransform SpawnTransform(
		SpawnRotation,
		SpawnLocation);

	AC_Zombie* SpawnedZombie =
		SpawnZombieFromPool(
			EZombieType::NormalZombie,
			SpawnTransform);

	if (!IsValid(SpawnedZombie))
	{
		UC_Util::Print(
			"From UC_ZombieManager::TestSpawnNormalZombie : Spawn Failed",
			FColor::Red,
			10.f);

		return false;
	}

	UC_Util::Print(
		"Zombie Spawn From Pool Success",
		FColor::Green,
		5.f);

	return true;
}
