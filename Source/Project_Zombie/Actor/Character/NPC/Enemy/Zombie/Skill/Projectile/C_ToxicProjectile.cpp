// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ToxicProjectile.h"

#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/RotationMatrix.h"

#include "Utility/C_Util.h"

#include "Actor/Character/NPC/Enemy/Zombie/Skill/ToxicAttack/C_ToxicPool.h"


AC_ToxicProjectile::AC_ToxicProjectile()
{
}

void AC_ToxicProjectile::SpawnToxicPool(const FVector& _SpawnLocation, const FRotator& _SpawnRotation)
{
	// 장판은 서버에서만 생성
	if (!HasAuthority())
		return;

	if (!m_ToxicPoolClass)
		return;

	UWorld* World = GetWorld();

	if (!World)
		return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = m_SkillUser;
	SpawnParams.Instigator = m_SkillUser;

	World->SpawnActor<AC_ToxicPool>(m_ToxicPoolClass, _SpawnLocation, _SpawnRotation, SpawnParams);

}

void AC_ToxicProjectile::ReachTarget()
{
	SpawnPoolAtGround(m_TargetLocation, nullptr);
}

void AC_ToxicProjectile::OnHit(AActor* _OtherActor, UPrimitiveComponent* _OtherCom, const FHitResult& _Hit)
{
	SpawnPoolAtGround(_Hit.ImpactPoint, _OtherActor);
}

bool AC_ToxicProjectile::FindPoolGround(const FVector& _ImpactLocation, AActor* _Target, FVector& _OutSpawnLocation, FRotator& _OutSpawnRotation) const
{
	UWorld* World = GetWorld();

	if (!World)
		return false;

	const float TraceStartOffset = 100.f;
	const float TraceDistance = 1000.f;

	const FVector TraceStart = _ImpactLocation + FVector::UpVector * TraceStartOffset;
	const FVector TraceEnd = _ImpactLocation - FVector::UpVector * TraceDistance;

	FHitResult GroundHit;
	
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ToxicPoolGroundTrace), false);

	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(m_SkillUser);

	// 플레이어를 라인트레이스 검사에서 제외
	if (IsValid(_Target))
	{
		QueryParams.AddIgnoredActor(_Target);
	}

	// 바닥같은 WorldStatic만 검사
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);

	// 라인트레이스 검사항목
	const bool bHitGround = World->LineTraceSingleByObjectType(GroundHit, TraceStart, TraceEnd, ObjectParams, QueryParams);

	// 디버그 라인 표시
	DrawDebugLine(World, TraceStart, TraceEnd, bHitGround ? FColor::Green : FColor::Red, false, 3.f, 0, 2.f);

	if (!bHitGround)
		return false;

	const float MinimumGroundNormalZ = 0.65f;

	if (GroundHit.ImpactNormal.Z < MinimumGroundNormalZ)
		return false;

	// 바닥에 닿았을 때 디버그 스피어 표시
	DrawDebugSphere(World, GroundHit.ImpactPoint, 15.f, 12, FColor::Yellow, false, 3.f);

	_OutSpawnLocation = GroundHit.ImpactPoint + GroundHit.ImpactNormal * 3.f;

	_OutSpawnRotation = FRotationMatrix::MakeFromZ(GroundHit.ImpactNormal).Rotator();

	return true;
}

void AC_ToxicProjectile::SpawnPoolAtGround(const FVector& _ImpactLocation, AActor* _Target)
{
	if (m_bFinished)
		return;

	m_bFinished = true;

	FVector PoolSpawnLocation;
	FRotator PoolSpawnRotation;

	const bool bFoundGround = FindPoolGround(_ImpactLocation, _Target, PoolSpawnLocation, PoolSpawnRotation);

	if (bFoundGround)
	{
		UC_Util::Print("!! PoolGround Found !!");

		SpawnToxicPool(PoolSpawnLocation, PoolSpawnRotation);
	}
	else
	{
		UC_Util::Print("PoolGround not Found");
	}

	Destroy();
}
