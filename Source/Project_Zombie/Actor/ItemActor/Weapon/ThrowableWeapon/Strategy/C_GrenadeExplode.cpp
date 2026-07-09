// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GrenadeExplode.h"
#include "../C_ThrowableWeaponBase.h"


bool UC_GrenadeExplode::UseStrategy_Implementation(AC_ThrowableWeaponBase* _ThrowableWeapon)
{
	if (!_ThrowableWeapon)
		return false;

	// 폭발이 발생한 위치
	const FVector ExplosionLocation = _ThrowableWeapon->GetActorLocation();

	// 폭발 반경
	const float ExplosionRadius = _ThrowableWeapon->GetExplosionRadius();

	// 폭발로 인한 최대 데미지
	const float MaxDamage = _ThrowableWeapon->GetMaxDamage();

	// 폭발로 인한 최소 데미지
	const float MinDamage = _ThrowableWeapon->GetMinDamage();

	// 폭발 데미지 적용 제외 액터 설정
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(_ThrowableWeapon);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoreActors);

	// 단순 충돌 체크
	QueryParams.bTraceComplex = false;

	// 충돌 결과를 저장할 배열
	TArray<FHitResult> HitResults;

	// Start 와 End 가 완전히 동일하면 SweepMultiByChannel은 충돌이 이상하게 나올 수 있다고 하네요
	// 그래서 End를 Z축으로 살짝 올림
	const FVector Start = ExplosionLocation;
	const FVector End = ExplosionLocation + FVector(0.f, 0.f, 1.f);

	const bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,	// 회전 없음
		ECC_Pawn,			// 충돌 채널 설정 /// 나중에 좀비 추가하면 변경
		FCollisionShape::MakeSphere(ExplosionRadius), // 충돌 범위 설정
		QueryParams			// 충돌 무시 등의 옵션
	);

	// ------------ 디버그용 ------------- 
	DrawDebugSphere(
		GetWorld(),
		ExplosionLocation,
		ExplosionRadius,
		32,
		FColor::Red,
		false,
		2.0f
	);
	// -----------------------------------

	// 아무도 충돌하지 않으면 폭발만 발생
	if (!bHit)
		return true; 
	
	
	return false;
}
