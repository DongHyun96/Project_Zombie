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

void AC_ToxicProjectile::OnHit(AActor* _OtherActor, UPrimitiveComponent* _OtherCom, const FHitResult& _Hit)
{
	FVector PoolSpawnLocation;
	FRotator PoolSpawnRotation;

	const bool bFoundGround = FindPoolGround(_Hit.ImpactPoint,_Hit.ImpactNormal, PoolSpawnLocation, PoolSpawnRotation);

	if (bFoundGround)
	{
		UC_Util::Print("!! PoolGround Found !!");

		SpawnToxicPool(PoolSpawnLocation, PoolSpawnRotation);
	}
	else
	{
		UC_Util::Print("PoolGround not Found");
	}

	Super::OnHit(_OtherActor, _OtherCom, _Hit);
}

bool AC_ToxicProjectile::FindPoolGround(const FVector& _ImpactLocation, const FVector& _ImpactNormal, FVector& _OutSpawnLocation, FRotator& _OutSpawnRotation) const
{
	UWorld* World = GetWorld();

	if (!World)
		return false;

	const float TraceStartOffset = 50.f;
	const float TraceDistance = 500.f;
	const float SurfaceIOffset = 20.f;

	const FVector TraceStart = _ImpactLocation + _ImpactNormal * SurfaceIOffset + FVector(0.f, 0.f, TraceStartOffset);
	const FVector TraceEnd = TraceStart - FVector(0.f, 0.f, TraceDistance);

	FHitResult GroundHit;
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const bool bHitGround = World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

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