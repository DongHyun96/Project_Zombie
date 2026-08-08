#include "C_FireDamageArea.h"

#include "Kismet/GameplayStatics.h"

#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"

#include "Components/AudioComponent.h"
#include "Engine/OverlapResult.h"

#include "Utility/C_Util.h"

AC_FireDamageArea::AC_FireDamageArea()
{
	// Tick 이 아니라 Timer 를 사용하므로 비활성화
	PrimaryActorTick.bCanEverTick = false;

	SetReplicates(true);
	SetReplicateMovement(false);
	bAlwaysRelevant = false;

	m_FirePatchEffectScale = 1.f;

	m_SpreadDirectionCount = 8;
	m_PatchDirectionCount = 3;
	m_PatchSpacing = 100.f;
	m_PatchDamageRadius = 100.f;

	m_ObstacleTraceHeight = 50.f;
	m_GroundTraceDistance = 150.f;
	m_GroundNormalZ = 0.5f;

	m_DamageHalfHeight = 150.f;
	m_DamageRadius = 300.f;
	m_Duration = 5.f;
	m_DamagePerSecond = 10.f;
	m_DamageInterval = 0.5f;

	// 가려지는 부분은 물리충돌을 해서 장판 생성 안되게 해야할 것 같기도...
	// 일단은 Collision 비활성화 처리
	SetActorEnableCollision(false);
}

void AC_FireDamageArea::BeginPlay()
{
	Super::BeginPlay();

	// 장판 생성
	GenerateFirePatches();

	// 장판 생성 시 사운드 재생
	if (m_FireSound)
	{
		m_FireAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
			GetWorld(),
			m_FireSound,
			GetActorLocation(),
			GetActorRotation(),
			1.f,	// Volume
			1.f,	// Pitch
			0.f,	// StartTime
			m_FireSoundAttenuation
		);
	}

	// 서버에서 수명 & 데미지 관리
	if (!HasAuthority())
		return;

	// 장판 생성 후 데미지 적용
	ApplyPointDamage();

	// 데미지 한번 적용 이후 그 이후부터는 타이머를 통해 반복 도트딜 적용
	GetWorldTimerManager().SetTimer(
		m_DamageTimerHandle,
		this,	// 타이머를 호출할 객체
		&AC_FireDamageArea::ApplyPointDamage, 
		m_DamageInterval,
		true	// 반복
	);

	// 장판 지속 시간 이후 장판 제거
	SetLifeSpan(m_Duration);
}

void AC_FireDamageArea::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 장판 제거 시 타이머 제거
	GetWorldTimerManager().ClearTimer(m_DamageTimerHandle);

	// 화염 장판 제거 시 사운드 제거
	if (IsValid(m_FireAudioComponent))
	{
		m_FireAudioComponent->Stop();
		m_FireAudioComponent->DestroyComponent();
	}

	// 장판 제거 시 파티클 제거
	for (UParticleSystemComponent* Effect : m_FirePatchEffects)
	{
		if (!IsValid(Effect))
			continue;

		Effect->DeactivateSystem();
		Effect->DestroyComponent();
	}

	// 장판 제거 시 장판 정보 제거
	m_FirePatchEffects.Empty();
	m_FirePatchInfos.Empty();
	
	// 왜 마지막에 호출하지
	Super::EndPlay(EndPlayReason);
}

bool AC_FireDamageArea::FindGroundAtLocation(const FVector& _StartLocation, FHitResult& _OutGroundHit)
{
	UWorld* World = GetWorld();
	if (!World)
		return false;

	constexpr float GroundTraceUpHeight = 5.f;

	const FVector StartTrace = _StartLocation + FVector(0.f, 0.f, GroundTraceUpHeight);
	const FVector EndTrace = _StartLocation - FVector(0.f, 0.f, m_GroundTraceDistance);

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(FindGroundAtLocation),
		false,	// 단순 콜리전 검사
		this	// Trace 검사에서 자기 자신을 무시
	);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	const bool bHitGround = World->LineTraceSingleByObjectType(
		_OutGroundHit,
		StartTrace,
		EndTrace,
		ObjectQueryParams,	// 지형과 충돌하는 채널
		QueryParams
	);

	// ============== 디버그용 =============
	if (m_bDebugDraw)
	{
		DrawDebugLine(
			World,
			StartTrace,
			EndTrace,
			bHitGround ? FColor::Green : FColor::Red,
			false,
			m_Duration,
			0,
			1.f
		);
	}
	// ====================================

	if (!bHitGround)
		return false;

	// 지형과 충돌했지만, 충돌한 표면이 가파르면 장판 생성 불가
	if (_OutGroundHit.ImpactNormal.Z < m_GroundNormalZ)
		return false;

	return true;
}

bool AC_FireDamageArea::IsSpreadBlock(const FFirePatchInfo& _FromPatch, const FFirePatchInfo& _EndPatch)
{
	UWorld* World = GetWorld();
	if (!World)
		return false;
	
	const FVector StartTrace = _FromPatch.PatchLocation + (_FromPatch.PatchNormal * m_ObstacleTraceHeight);
	const FVector EndTrace = _EndPatch.PatchLocation + (_EndPatch.PatchNormal * m_ObstacleTraceHeight);

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(FireSpreadObstacleTrace),
		false,	// 단순 콜리전 검사
		this	// Trace 검사에서 자기 자신을 무시
	);

	FHitResult ObstacleHit;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	const bool bBlocked = World->LineTraceSingleByObjectType(
		ObstacleHit,
		StartTrace,
		EndTrace,
		ObjectQueryParams,	// 지형과 충돌하는 채널 /// 나중에 추가해야함
		QueryParams
	);

	// ============== 디버그용 =============
	if (m_bDebugDraw)
	{
		DrawDebugLine(
			World,
			StartTrace,
			EndTrace,
			bBlocked ? FColor::Red : FColor::Green,
			false,
			m_Duration,
			0,
			1.f
		);
	}
	// ====================================

	return bBlocked;
}

void AC_FireDamageArea::AddFirePatch(const FFirePatchInfo& _Patch)
{
	m_FirePatchInfos.Add(_Patch);

	if (!m_FirePatchEffect)
		return;

	UParticleSystemComponent* SpawnEffect = UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		m_FirePatchEffect,
		_Patch.PatchLocation,
		_Patch.PatchNormal.Rotation(),
		FVector(m_FirePatchEffectScale)
	);

	if (SpawnEffect)
	{
		m_FirePatchEffects.Add(SpawnEffect);
	}
}

bool AC_FireDamageArea::IsTargetInFireArea(const FVector& _TargetLocation)
{
	return false;
}

void AC_FireDamageArea::GenerateFirePatches()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// 장판 생성 시 기존 장판 정보 제거
	m_FirePatchInfos.Empty();

	// ============= 중심 위치의 실제 바닥 찾기 =============

	FHitResult CenterGroundHit;

	if (!FindGroundAtLocation(GetActorLocation(), CenterGroundHit))
		return;

	FFirePatchInfo CenterPatch;

	CenterPatch.PatchNormal = CenterGroundHit.ImpactNormal;
	CenterPatch.PatchLocation = CenterGroundHit.ImpactPoint + (CenterPatch.PatchNormal * 0.2f);

	AddFirePatch(CenterPatch);

	// GPT 사용;; 
	// ============== 중심 위치에서 방향 나누기 =============

	const float AngleStep = 360.f / m_SpreadDirectionCount;

	for (int32 DirectionIndex = 0; DirectionIndex < m_SpreadDirectionCount; ++DirectionIndex)
	{
		const float Angle = DirectionIndex * AngleStep;

		// 현재 각도의 수평 확산 방향 벡터 계산
		const FVector DirectionVector(
			FMath::Cos(FMath::DegreesToRadians(Angle)),
			FMath::Sin(FMath::DegreesToRadians(Angle)),
			0.f
		);

		// 중심 장판에서 한 방향으로 퍼지는 장판 생성
		FFirePatchInfo PreviousPatch = CenterPatch;

		// ================= 중심 장판부터 한 칸씩 확장 =================

		for (int32 PatchIndex = 1; PatchIndex <= m_PatchDirectionCount; ++PatchIndex)
		{
			// 중심 장판에서 현재 방향으로 일정 간격 떨어진 위치 계산
			const FVector PatchLocation = CenterPatch.PatchLocation + (DirectionVector * m_PatchSpacing * PatchIndex);
			
			// 바닥 찾기
			FHitResult GroundHit;
			if (!FindGroundAtLocation(PatchLocation, GroundHit))
			{
				break;
			}

			// 바닥 발견
			FFirePatchInfo NewPatch;
			NewPatch.PatchNormal = GroundHit.ImpactNormal;
			NewPatch.PatchLocation = GroundHit.ImpactPoint + (NewPatch.PatchNormal * 0.2f);

			// ================ 이전 장판과 현재 장판 사이에 벽이 있는지 확인 ================

			// 벽 찾기
			if (IsSpreadBlock(PreviousPatch, NewPatch))
			{
				break;
			}

			// 벽이 없으면 장판 정보 추가
			AddFirePatch(NewPatch);

			// 다음 장판을 위해 이전 장판 갱신
			// 이제 다음 장판을 찾을 때 이전 장판과 현재 장판 사이에 벽이 있는지 확인해야 하므로
			PreviousPatch = NewPatch;
		}
	}
}

void AC_FireDamageArea::ApplyPointDamage()
{
	// 실제 장판 데미지는 서버에서만 처리
	if (!HasAuthority())
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	// 화염병 던진 플레이어 Controller
	AController* InstigatorController = GetInstigatorController();

	const FVector AreaLocation = GetActorLocation();

	// 찾을 액터의 충돌 채널 설정
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn); // 나중에 좀비 추가하면 변경

	// 자기 자신 제외
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);


	// 충돌 결과를 저장할 배열
	TArray<FOverlapResult> OverlapResults;

	// 1. 장판 반경 안의 대상 검색
	const FCollisionShape CollisionShape = 
		FCollisionShape::MakeBox(
			FVector(
				m_DamageRadius,
				m_DamageRadius,
				m_DamageHalfHeight			// Z축
			)
		);

	const bool bHasOverlap = World->OverlapMultiByObjectType
	(
		OverlapResults,
		AreaLocation,
		FQuat::Identity,
		ObjectQueryParams,
		CollisionShape,
		QueryParams
	);

	// -------- 디버그용 --------------
	if (m_bDebugDraw)
	{
		DrawDebugCylinder
		(
			World,
			AreaLocation - FVector(0.f, 0.f, m_DamageHalfHeight),
			AreaLocation + FVector(0.f, 0.f, m_DamageHalfHeight),
			m_DamageRadius,
			32,
			FColor::Red,
			false,
			m_DamageInterval,
			0,
			2.f
		);
	}
	// -------------------------------

	// 아무도 충돌하지 않으면 폭발만 발생
	if (!bHasOverlap)
		return;

	// 중복 제거를 위해 Set 사용
	TSet<AActor*> DamagedActors;


	//TSubclassOf<UDamageType> AppliedDamageType =
	//	m_DamageTypeClass;


	//if (!AppliedDamageType)
	//{
	//	AppliedDamageType =
	//		UDamageType::StaticClass();
	//}

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* Target = Result.GetActor();

		if (!IsValid(Target))
			continue;

		if (!Target->CanBeDamaged())
			continue;

		// 이미 데미지를 입은 타겟은 건너뜀
		if (DamagedActors.Contains(Target))
			continue;

		// 2. 위치 구하기
		FVector TargetLocation = Target->GetActorLocation();

		const FVector2D HorizontalOffset(
			TargetLocation.X - AreaLocation.X,
			TargetLocation.Y - AreaLocation.Y
		);

		// 거리 제곱값 계산
		const float HorizontalDistanceSquared =
			HorizontalOffset.SizeSquared();

		if (HorizontalDistanceSquared >
			FMath::Square(m_DamageRadius))
		{
			continue;
		}

		const float Damage = m_DamagePerSecond * m_DamageInterval;

		UGameplayStatics::ApplyDamage(
			Target,
			Damage,
			InstigatorController,
			this,
			UDamageType::StaticClass()
		);

		// 중복방지 위해 데미지 입은 액터 Set 에 추가
		DamagedActors.Add(Target);

		UE_LOG
		(
			LogTemp,
			Warning,
			TEXT("[Fire Area Damage] Target=%s / Damage=%.2f"),
			*GetNameSafe(Target),
			Damage
		);
	}
}

