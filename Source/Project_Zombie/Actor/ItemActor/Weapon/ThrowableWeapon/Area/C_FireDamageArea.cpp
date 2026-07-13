#include "C_FireDamageArea.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"

#include "../../../../Character/Player/C_BasicPlayer.h"
#include "Engine/OverlapResult.h"

#include "Utility/C_Util.h"

AC_FireDamageArea::AC_FireDamageArea()
{
	// Tick 이 아니라 Timer 를 사용하므로 비활성화
	PrimaryActorTick.bCanEverTick = false;

	m_FireEffectComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireEffect"));
	SetRootComponent(m_FireEffectComponent);
	m_FireEffectComponent->SetAutoActivate(true);

	// 가려지는 부분은 물리충돌을 해서 장판 생성 안되게 해야할 것 같기도...
	// 일단은 Collision 비활성화 처리
	SetActorEnableCollision(false);
}

void AC_FireDamageArea::BeginPlay()
{
	Super::BeginPlay();

	m_DamageRadius = 300.f;
	m_Duration = 5.f;
	m_DamagePerSecond = 10.f;
	m_DamageInterval = 1.f;

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
	Super::EndPlay(EndPlayReason);
}

void AC_FireDamageArea::ApplyPointDamage()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

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
				50.f			// Z축
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
	DrawDebugCylinder(
		World,
		AreaLocation + FVector(0.f, 0.f, 2.f),	// Start
		AreaLocation + FVector(0.f, 0.f, 5.f),	// End
		m_DamageRadius,
		32,
		FColor::Red,
		false,
		m_DamageInterval,
		0,
		2.f
	);
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

		if (!Target)
			continue;

		if (!Target->CanBeDamaged())
			continue;

		// 이미 데미지를 입은 타겟은 건너뜀
		if (DamagedActors.Contains(Target))
			continue;

		// 2. 위치 구하기
		FVector TargetLocation = Target->GetActorLocation();


		/*
		 * 대상과 장판 중심 사이의 XY 차이만 구한다.
		 *
		 * Z축 높이는 현재 단계에서 검사하지 않는다.
		 */
		const FVector2D HorizontalOffset(
			TargetLocation.X -
			AreaLocation.X,

			TargetLocation.Y -
			AreaLocation.Y
		);


		const float HorizontalDistanceSquared =
			HorizontalOffset.SizeSquared();


		/*
		 * XY 평면상에서 원형 장판 반경 밖이면 제외
		 *
		 * 실제 거리 계산에 제곱근을 사용하지 않도록
		 * 거리 제곱값끼리 비교한다.
		 */
		if (HorizontalDistanceSquared >
			FMath::Square(m_DamageRadius))
		{
			continue;
		}


		// 3. 데미지 적용 
		/// InstigatorController 는 어떻게 가져올지 고민 필요
		UGameplayStatics::ApplyDamage(
			Target,
			m_DamagePerSecond * m_DamageInterval,	// 초당 데미지 * 간격
			nullptr,								// InstigatorController
			this,									// DamageCauser
			m_DamageType							// DamageTypeClass
		);

		// 데미지를 입은 액터를 Set에 추가하여 중복 방지
		DamagedActors.Add(Target);

		UC_Util::Print("[AC_FireDamageArea::ApplyPointDamage] Hit");
	}
}

