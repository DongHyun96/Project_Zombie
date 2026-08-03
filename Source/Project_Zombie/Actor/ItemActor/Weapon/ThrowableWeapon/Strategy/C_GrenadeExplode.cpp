// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GrenadeExplode.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

#include "../../../../Character/Player/C_BasicPlayer.h"
#include "../C_ThrowableWeaponBase.h"

#include "Utility/C_Util.h"


bool UC_GrenadeExplode::UseStrategy_Implementation(AC_ThrowableWeaponBase* _ThrowableWeapon)
{
	if (!_ThrowableWeapon)
		return false;

	if (!_ThrowableWeapon->HasAuthority())
		return false;

	// 폭발이 발생한 위치
	const FVector ExplosionLocation = _ThrowableWeapon->GetActorLocation();

	// 폭발 반경
	const float ExplosionRadius = _ThrowableWeapon->GetExplosionRadius();

	// 폭발로 인한 최대 데미지
	const float MaxDamage = _ThrowableWeapon->GetMaxDamage();

	// 폭발로 인한 최소 데미지
	const float MinDamage = _ThrowableWeapon->GetMinDamage();


	// 찾을 액터의 충돌 채널 설정
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn); // 나중에 좀비 추가하면 변경


	FCollisionQueryParams QueryParams;

	// 폭발 데미지 적용 제외 액터 설정
	//TArray<AActor*> IgnoreActors;
	//IgnoreActors.Add();
	//QueryParams.AddIgnoredActors(IgnoreActors);
	
	// 자기 자신 제외
	QueryParams.AddIgnoredActor(_ThrowableWeapon);

	// 단순 충돌
	QueryParams.bTraceComplex = false;


	/*
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
	*/

	// 충돌 결과를 저장할 배열
	TArray<FOverlapResult> OverlapResults;

	// 1. 폭발 반경 안의 대상 검색
	const bool bHasOverlap = GetWorld()->OverlapMultiByObjectType
	(
		OverlapResults,
		ExplosionLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(ExplosionRadius),
		QueryParams
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
	if (!bHasOverlap)
		return true; 

	// 중복 제거를 위해 Set 사용
	TSet<AActor*> DamagedActors;
	
	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* Target = Result.GetActor();
		
		if (!Target)
			continue;

		// 이미 데미지를 입은 타겟은 건너뜀
		if (DamagedActors.Contains(Target))
			continue;
		
		// 2. 위치 구하기
		FVector TargetLocation = Target->GetActorLocation();

		// 폭발 위치와 타겟 위치 거리
		const float Distance = FVector::Distance(ExplosionLocation, TargetLocation);

		if (Distance > ExplosionRadius)
			continue;

		// 3. 가려짐 확인
		FHitResult BlockHit;

		// LineTrace 옵션 설정
		FCollisionQueryParams TraceParams;
		
		// 자기자신 무시
		TraceParams.AddIgnoredActor(_ThrowableWeapon);

		// TODO
		// 나중에 바닥도 무시할 수 있도록 추가

		// 몸체 중심쪽 검사를 위해 Z축을 살짝 올려서 Trace 시작점과 끝점을 설정
		const FVector TraceStart = ExplosionLocation + FVector(0.f, 0.f, 100.f);
		const FVector TraceEnd = TargetLocation + FVector(0.f, 0.f, 100.f);

		// LineTrace 사용하여 폭발 위치와 타겟 위치 사이에 장애물이 있는지 확인
		const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
			BlockHit,										// 결과 저장
			TraceStart,
			TraceEnd,
			_ThrowableWeapon->GetExplosionTraceChannel(),	// Trace Channel 설정
			TraceParams										// Trace 옵션
		);


		// ----------- TraceColor 는 디버그용 -------------
		// Trace 가 막혔으면 데미지 적용하지 않음 (자기자신 제외)
		if (bBlocked && BlockHit.GetActor() != Target)
		{
			const FColor TraceColor = FColor::Red;
			DrawDebugLine(
				GetWorld(),
				TraceStart,
				TraceEnd,
				TraceColor,
				false,
				2.0f,
				0,
				1.5f
			);

			continue;
		}


		const FColor TraceColor = FColor::Green;
		DrawDebugLine(
			GetWorld(),
			TraceStart,
			TraceEnd,
			TraceColor,
			false,
			2.0f,
			0,
			1.5f
		);
		// -----------------------------------



		// 4. 데미지를 입히기
		AController* InstigatorController = nullptr;
		if (_ThrowableWeapon->GetOwnerPlayer())
		{
			InstigatorController = _ThrowableWeapon->GetOwnerPlayer()->GetController();
		}

		UE_LOG
		(
			LogTemp,
			Warning,
			TEXT(
				"[Grenade Damage] Target=%s / Distance=%.2f / Radius=%.2f / Max=%.2f / Min=%.2f"
			),
			*GetNameSafe(Target),
			Distance,
			ExplosionRadius,
			MaxDamage,
			MinDamage
		);




		// 데미지 이벤트 전달
		UGameplayStatics::ApplyDamage(
			Target,						// 데미지 받는 대상
			FMath::Lerp(MaxDamage, MinDamage, Distance / ExplosionRadius), // 거리 비례 데미지 계산
			InstigatorController,		// 데미지를 입힌 주체
			_ThrowableWeapon,			// 데미지를 입힌 무기
			UDamageType::StaticClass()	// 데미지 타입
		);

		// 5. 데미지를 입은 액터를 Set에 추가하여 중복 방지
		DamagedActors.Add(Target);

		UC_Util::Print("[UC_GrenadeExplode::UseStrategy_Implementation] Hit");
	}
	
	return true;
}
