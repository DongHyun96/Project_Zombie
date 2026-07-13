// Fill out your copyright notice in the Description page of Project Settings.


#include "C_MolotovExplode.h"

#include "../C_ThrowableWeaponBase.h"
#include "../Area/C_FireDamageArea.h"

#include "Utility/C_Util.h"

bool UC_MolotovExplode::UseStrategy_Implementation(AC_ThrowableWeaponBase* _ThrowableWeapon)
{
	if (!_ThrowableWeapon)
		return false;

	// 화염 장판 클래스 가져오기
	const TSubclassOf<AC_FireDamageArea> FireDamageAreaClass = _ThrowableWeapon->GetFireDamageAreaClass();

	if (!FireDamageAreaClass)
	{
		UC_Util::Print("[C_MolotovExplode] FireDamageAreaClass is not assigned");
		return false;
	}

	// 폭발이 발생한 위치
	FVector ExplosionLocation = _ThrowableWeapon->GetActorLocation();

	// 화염병이 벽에 충돌했을 수도 있으므로 아래로 Trace 검사
	const FHitResult& ImpactHit = _ThrowableWeapon->GetHitResult();
	if (ImpactHit.bBlockingHit)
	{
		// ImpactNormal : 충돌한 표면에 수직인 벡터
		const FVector ImpactNormal = ImpactHit.ImpactNormal;

		if (!ImpactNormal.IsNearlyZero())
		{
			// 충돌 지점에서 약간 떨어진 위치로 조정
			ExplosionLocation = ImpactHit.ImpactPoint + ImpactNormal * 10.f;
		}
		else
		{
			// 0 벡터인 경우, 충돌 지점을 그대로 사용
			ExplosionLocation = ImpactHit.ImpactPoint;
		}
	}

	// Trace를 통해 화염 장판을 생성할 위치를 결정
	const FVector GroundTraceStart = ExplosionLocation + FVector(0.f, 0.f, 50.f);
	const FVector GroundTraceEnd = ExplosionLocation - FVector(0.f, 0.f, 200.f);

	FCollisionQueryParams GroundQueryParams(
		SCENE_QUERY_STAT(MolotovGroundTrace), // 디버그용 이름
		false,	// 단순 콜리전 검사
		_ThrowableWeapon // Trace 검사에서 자기 자신을 무시
	);

	FHitResult GroundHit;

	const bool bHitGround = GetWorld()->LineTraceSingleByChannel(
		GroundHit,
		GroundTraceStart,
		GroundTraceEnd,
		ECC_Visibility, // Trace 채널 설정
		GroundQueryParams
	);

	// 바닥 못찾으면 장판은 생성 안하고 폭발 효과만 발생시키고 끝냄
	if (!bHitGround)
	{
		return true;
	}

	AC_FireDamageArea* FireDamageArea = GetWorld()->SpawnActor<AC_FireDamageArea>(
		FireDamageAreaClass,
		GroundHit.ImpactPoint, // 바닥 충돌 지점에 장판 생성
		FRotator::ZeroRotator  // 회전 없음
	);

	return true;
}
