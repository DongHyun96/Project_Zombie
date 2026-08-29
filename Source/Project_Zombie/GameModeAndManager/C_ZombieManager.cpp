// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ZombieManager.h"

#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "Actor/Character/NPC/Enemy/Zombie/NurseZombie/C_NurseZombie.h"
#include "Actor/Character/NPC/Enemy/Zombie/NurseZombie/C_HealingProjectile.h"
#include "Actor/Character/NPC/Enemy/Zombie/Spawn/C_SpawnArea.h"

#include "Components/CapsuleComponent.h"
#include "Utility/C_Util.h"

UC_ZombieManager::UC_ZombieManager()
{
	// _C 없이 애셋 경로만
	static ConstructorHelpers::FClassFinder<AC_Zombie> NurseFinder(TEXT("/Game/DongHyun/Actor/Enemy/BP_NurseZombie"));

	if (NurseFinder.Succeeded())
		m_ZombieClasses.Add(EZombieType::NurseZombie, NurseFinder.Class);

	static ConstructorHelpers::FClassFinder<AC_Zombie> NormalFinder(TEXT("/Game/Harang/BP/Zombie/BP_NormalZombie"));

	if (NormalFinder.Succeeded())
		m_NormalZombieClasses.Add(NormalFinder.Class);

	static ConstructorHelpers::FClassFinder<AC_Zombie> Normal2Finder(TEXT("/Game/Harang/BP/Zombie/BP_NormalZombie2"));

	if (Normal2Finder.Succeeded())
		m_NormalZombieClasses.Add(Normal2Finder.Class);

	static ConstructorHelpers::FClassFinder<AC_Zombie> Normal3Finder(TEXT("/Game/Harang/BP/Zombie/BP_NormalZombie3"));

	if (Normal3Finder.Succeeded())
		m_NormalZombieClasses.Add(Normal3Finder.Class);

	static ConstructorHelpers::FClassFinder<AC_Zombie> PoisonFinder(TEXT("/Game/Harang/BP/Zombie/BP_ToxicZombie"));

	if (PoisonFinder.Succeeded())
		m_ZombieClasses.Add(EZombieType::ToxicZombie, PoisonFinder.Class);

	static ConstructorHelpers::FClassFinder<AC_Zombie> TankFinder(TEXT("/Game/Harang/BP/Zombie/BP_TankZombie"));

	if (TankFinder.Succeeded())
		m_ZombieClasses.Add(EZombieType::TankZombie, TankFinder.Class);

	for (uint8 i = 0; i < static_cast<uint8>(EZombieType::End); ++i)
	{
		EZombieType ZombieType = static_cast<EZombieType>(i);
		m_ActiveZombies.Add(ZombieType, {});
	}
}

void UC_ZombieManager::OnWorldBeginPlay()
{
	// TODO : 실질적인 Object pooling 해두기
	UWorld* World = GetWorld();

	// 서버에서만 실행
	if (!IsValid(World) || World->GetNetMode() == NM_Client)
		return;

	// 게임 시작 시 초기 좀비 풀 미리 생성
	InitializeZombiePool();

	// HealingProjectile Pooling 처리
	if (m_HealingProjectileClass)
	{
		for (int i = 0; i < 45; ++i)
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

void UC_ZombieManager::InitializeZombiePool()
{
	UWorld* World = GetWorld();

	// 서버에서만 실행
	if (!IsValid(World) || World->GetNetMode() == NM_Client)
		return;

	// 타입별 PoolCount 순회
	for (const TPair<EZombieType, uint32>& PoolInfo : m_PoolCounts)
	{
		const EZombieType ZombieType = PoolInfo.Key;
		
		const uint32 PoolCount = PoolInfo.Value;

		// 생성 수가 0이면 처리 x
		if (PoolCount == 0)
			continue;

		// 해당 타입의 대기 풀 가져오기
		TArray<AC_Zombie*>& pPool = m_ZombiePool.FindOrAdd(ZombieType);

		// 설정된 수만큼 생성
		for (uint32 i = 0; i < PoolCount; ++i)
		{
			// ======== 생성할 좀비 클래스 결정 ========
			TSubclassOf<AC_Zombie> SpawnClass;

			// NormalZombie는 랜덤외형 선택
			if (ZombieType == EZombieType::NormalZombie && !m_NormalZombieClasses.IsEmpty())
			{
				const int32 RandomIndex = FMath::RandRange(0, m_NormalZombieClasses.Num() - 1);

				SpawnClass = m_NormalZombieClasses[RandomIndex];
			}

			// 그 외의 좀비들
			else
			{
				const TSubclassOf<AC_Zombie>* ZombieClass = m_ZombieClasses.Find(ZombieType);
				if (!ZombieClass || !(*ZombieClass))
				{
					UC_Util::Print(
						"From InitializeZombiePool : ZombieClass not found",
						FColor::Red,
						10.f);

					continue;
				}

				SpawnClass = *ZombieClass;
			}

			// SpawnClass가 유효한지 확인
			if (!SpawnClass)
			{
				UC_Util::Print("From InitializeZombiePool : SpawnClass not found", FColor::Red, 10.f);
			}

			FActorSpawnParameters SpawnParams;

			// 초기 풀 객체는 생성 직후 바로 숨길거라서
			// 생성 위치의 충돌 여부와 관계없이 강제로 생성
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// 생성 즉시 Deactivateforpool 에서 숨김처리와 충돌처리를 끄기때문에
			// 월드 원점에 생성되도 무방
			// 나중에 ActivateFromPool 에서 실제 스폰위치로 Teleport하기 때문에 괜찮음
			// AC_Zombie* SpawnZombie = World->SpawnActor<AC_Zombie>(*ZombieClass, FTransform::Identity, SpawnParams);
			AC_Zombie* SpawnZombie = World->SpawnActorDeferred<AC_Zombie>
			(
				SpawnClass,
				FTransform::Identity,
				nullptr, nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn
			);
			
			if (!IsValid(SpawnZombie))
			{
				UC_Util::Print("From InitializeZombiePool : Zombie Spawn Failed !!", FColor::Red, 10.f);
				continue;
			}

			// 레벨에 사용자가 직접 넣은 좀비가 아님 -> ZombieManager에 의해 스폰된 ZombieActor임
			SpawnZombie->m_bIsLevelPlaced = false;
			
			// 위의 설정값 이후, BeginPlay 처리
			SpawnZombie->FinishSpawning(FTransform::Identity);
			
			// 생성된 좀비를 즉시 풀 대기상태로 전환
			if (!SpawnZombie->DeactivateForPool())
			{
				UC_Util::Print("From InitializeZombiePool : DeactivateForPool Failed !!", FColor::Red, 10.f);

				SpawnZombie->Destroy();
				continue;
			}
			
			// 비활성 대기 풀 등록
			pPool.Add(SpawnZombie);

		}
	}
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
		return nullptr;

	// 대기 풀에서 제거
	pPool->Pop();

	// 필드 활성 목록에 등록
	m_ActiveZombies[_ZombieType].Add(Zombie);
	
	return Zombie;
}

bool UC_ZombieManager::StartSpawnLoop(const TArray<AC_SpawnArea*>& _SpawnArea, const FZombieWaveSetting& _Settings)
{
	UWorld* World = GetWorld();

	// 웨이브 관리는 서버에서만 
	if (!IsValid(World) || World->GetNetMode() == NM_Client)
		return false;

	// SpawnArea가 없으면 시작할수 없음
	if (_SpawnArea.IsEmpty())
		return false;

	// 기존의 SpawnLoop가 진행 중이었다면 먼저 종료
	StopSpawnLoop();

	// 유효한 SpawnArea만 저장
	m_CurrentSpawnAreas.Reset();

	for (AC_SpawnArea* SpawnArea : _SpawnArea)
	{
		if (!IsValid(SpawnArea))
			continue;

		if (!SpawnArea->IsEnabled())
			continue;

		m_CurrentSpawnAreas.Add(SpawnArea);
	}

	if (m_CurrentSpawnAreas.IsEmpty())
		return false;

	// 이번 웨이브 설정저장
	m_CurrentWaveSetting = _Settings;

	// 이전 웨이브의 타입별 스폰 쿨타임 초기화
	m_NextZombieSpawnTime.Reset();

	m_bSpawnLoopActive = true;

	// 반복 타이머 시작
	World->GetTimerManager().SetTimer(m_SpawnLoopTimer, this, &UC_ZombieManager::HandleSpawnLoopTick,
										m_CurrentWaveSetting.SpawnInterval, true);

	return true;
}

void UC_ZombieManager::StopSpawnLoop()
{
	UWorld* World = GetWorld();

	if (IsValid(World))
	{
		World->GetTimerManager().ClearTimer(m_SpawnLoopTimer);
	}

	m_bSpawnLoopActive = false;

	m_CurrentSpawnAreas.Reset();
}

bool UC_ZombieManager::AddZombieToActivePoolManually(EZombieType _ZombieType, AC_Zombie* _Zombie)
{
	if (_ZombieType == EZombieType::End || !IsValid(_Zombie)) return false;
	m_ActiveZombies[_ZombieType].Add(_Zombie);
	return true;
}

void UC_ZombieManager::HandleSpawnLoopTick()
{
	// SpawnLoop가 중지된 상태
	if (!m_bSpawnLoopActive)
		return;

	// SpawnArea가 없는 경우
	if (m_CurrentSpawnAreas.IsEmpty())
		return;

	// Tick에서 설정된 수만큼 Spawn 시도
	for (int32 i = 0; i < m_CurrentWaveSetting.SpawnCountPerTick; ++i)
	{
		// 현재 스폰 가능한 타입 중 하나 선택
		const FZombieTypeSpawnSetting* SpawnSetting = SelectZombieTypeToSpawn();

		// 현재 스폰 가능한 타입 자체가 없음
		if (!SpawnSetting)
			break;

		// 해당 타입을 허용하는 SpawnArea 선택
		AC_SpawnArea* SpawnArea = SelectSpawnAreaForZombieType(SpawnSetting->ZombieType);

		if (!IsValid(SpawnArea))
			continue;

		// 실제 Spawn
		if (TrySpawnZombieFromArea(SpawnSetting->ZombieType, SpawnArea))
		{
			// 실제 spawn 성공했을 때만
			// 타입별 cooldown 시작
			StartZombieSpawnCooldown(*SpawnSetting);
		}
	}
}

bool UC_ZombieManager::CanSpawnZombieType(const FZombieTypeSpawnSetting& _Setting) const
{
	UWorld* World = GetWorld();

	if (!IsValid(World))
		return false;

	// 선택 가중치가 없으면 등장하지 않는 타입
	if (_Setting.SpawnWeight <= 0.f)
		return false;

	const int32 ActiveCount = GetActiveZombieCount(_Setting.ZombieType);
	
	// 타입별 최대 활성 수 검사
	if (ActiveCount >= _Setting.MaxActiveCount)
		return false;

	// 해당 타입 Pool이 존재하고 꺼낼 좀비가 남아있는지 확인
	const TArray<AC_Zombie*>* Pool = m_ZombiePool.Find(_Setting.ZombieType);

	if (!Pool)
		return false;

	if (Pool->IsEmpty())
		return false;
	
	// 타입별 SpawnCoolDown 검사
	if (const float* NextSpawnTime = m_NextZombieSpawnTime.Find(_Setting.ZombieType))
	{
		if (World->GetTimeSeconds() < *NextSpawnTime)
		{
			return false;
		}
	}

	AC_SpawnArea* SpawnArea = SelectSpawnAreaForZombieType(_Setting.ZombieType);
	
	// 해당 타입을 허용하는 SpawnArea도 있어야 함
	if (!IsValid(SpawnArea))
		return false;

	return true;
}


AC_SpawnArea* UC_ZombieManager::SelectSpawnAreaForZombieType(EZombieType _ZombieType) const
{
	// 해당 타입을 가지고 있는 Area 후보 목록
	TArray<AC_SpawnArea*> AvailableAreas;

	for (AC_SpawnArea* SpawnArea : m_CurrentSpawnAreas)
	{
		// 잘못된 Area 제외
		if (!IsValid(SpawnArea))
			continue;

		// 비활성화된 Area 제외
		if (!SpawnArea->IsEnabled())
			continue;

		// 해당 좀비 타입을 허용하지 않는 Area 제외
		if (!SpawnArea->IsZombieTypeAllowed(_ZombieType))
			continue;

		AvailableAreas.Add(SpawnArea);
	}
	
	// 사용 가능한 Area가 없을 시
	if (AvailableAreas.IsEmpty())
		return nullptr;

	// 후보 중 하나 랜덤 선택
	const int32 RandomIndex = FMath::RandRange(0, AvailableAreas.Num() - 1);

	return AvailableAreas[RandomIndex];
}

const FZombieTypeSpawnSetting* UC_ZombieManager::SelectZombieTypeToSpawn() const
{
	// 스폰 가능한 설정만
	TArray<const FZombieTypeSpawnSetting*> Candidates;

	float TotalWeight = 0.f;

	for (const FZombieTypeSpawnSetting& Setting : m_CurrentWaveSetting.ZombieTypeSetting)
	{
		if (!CanSpawnZombieType(Setting))
			continue;

		Candidates.Add(&Setting);
		TotalWeight += Setting.SpawnWeight;
	}

	// 후보가 하나도 없음
	if (Candidates.IsEmpty() || TotalWeight <= 0.f)
		return nullptr;

	// 0 ~ TotalWeight 사이 랜덤 값
	const float RandomWeight = FMath::FRandRange(0.f, TotalWeight);

	float AccumulatedWeight = 0.f;

	for (const FZombieTypeSpawnSetting* Setting : Candidates)
	{
		if (!Setting)
			continue;

		AccumulatedWeight += Setting->SpawnWeight;

		if (RandomWeight <= AccumulatedWeight)
			return Setting;
	}

	// 부동소수점 오차 방어
	return Candidates.Last();

}

void UC_ZombieManager::StartZombieSpawnCooldown(const FZombieTypeSpawnSetting& _Setting)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
		return;

	// Cooldown이 없는 타입은 기록안해도됨
	if (_Setting.SpawnCoolDown <= 0.f)
		return;

	const float NextSpawnTime = World->GetTimeSeconds() + _Setting.SpawnCoolDown;

	m_NextZombieSpawnTime.Add(_Setting.ZombieType, NextSpawnTime);
}

int32 UC_ZombieManager::GetActiveZombieCount(EZombieType _ZombieType) const
{
	const TSet<AC_Zombie*>* ActiveSet = m_ActiveZombies.Find(_ZombieType);

	if (!ActiveSet)
		return 0;

	return ActiveSet->Num();
}

bool UC_ZombieManager::TrySpawnZombieFromArea(EZombieType _ZombieType, AC_SpawnArea* _SpawnArea)
{
	UWorld* World = GetWorld();

	// 서버에서만 스폰 처리
	if (!IsValid(World) ||
		World->GetNetMode() == NM_Client)
	{
		return false;
	}

	// 전달받은 SpawnArea 유효성 검사
	if (!IsValid(_SpawnArea))
	{
		UC_Util::Print(
			"From TrySpawnZombieFromArea : SpawnArea is nullptr",
			FColor::Red,
			5.f);

		return false;
	}

	// 해당 SpawnArea에서 이 타입을 허용하는지 확인
	if (!_SpawnArea->IsZombieTypeAllowed(_ZombieType))
		return false;

	// 해당 좀비 클래스 가져오기
	TSubclassOf<AC_Zombie> ZombieClass;

	if (_ZombieType == EZombieType::NormalZombie && !m_NormalZombieClasses.IsEmpty())
	{
		// 노말좀비의 외형은 여러개지만
		// 위치검사는 캡슐만 필요하므로
		// 첫번째 좀비의 CDO 사용
		ZombieClass = m_NormalZombieClasses[0];
	}
	else
	{
		const TSubclassOf<AC_Zombie>* FoundClass = m_ZombieClasses.Find(_ZombieType);

		if (!FoundClass || !(*FoundClass))
		{
			UC_Util::Print(
				"From TrySpawnZombieFromArea : ZombieClass not found",
				FColor::Red,
				5.f);

			return false;
		}

		ZombieClass = *FoundClass;
	}
	

	// 실제 Spawn하지 않고 클래스 기본 객체(CDO)에서
	// 캡슐 크기만 가져오기
	const AC_Zombie* ZombieCDO = (*ZombieClass)->GetDefaultObject<AC_Zombie>();

	if (!IsValid(ZombieCDO))
		return false;

	const UCapsuleComponent* Capsule = ZombieCDO->GetCapsuleComponent();

	if (!IsValid(Capsule))
		return false;

	const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();

	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	// SpawnArea에게 안전한 위치 요청
	FTransform SpawnTransform;

	if (!_SpawnArea->FindValidSpawnTransform(
		_ZombieType,
		CapsuleRadius,
		CapsuleHalfHeight,
		SpawnTransform))
	{
		UC_Util::Print(
			"From TrySpawnZombieFromArea : Valid Spawn Transform not found",
			FColor::Red,
			5.f);

		return false;
	}

	// 기존 Pool에서 좀비 활성화
	return IsValid(SpawnZombieFromPool(_ZombieType, SpawnTransform));
}
