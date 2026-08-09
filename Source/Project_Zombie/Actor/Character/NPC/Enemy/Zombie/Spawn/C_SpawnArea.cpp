// Fill out your copyright notice in the Description page of Project Settings.


#include "C_SpawnArea.h"
#include "GameModeAndManager/C_GameMode_GameLv.h"
#include "GameModeAndManager/PointTowerManager/C_PointTowerManager.h"
#include "Components/BoxComponent.h"
#include "NavigationSystem.h"
#include "Utility/C_Util.h"

AC_SpawnArea::AC_SpawnArea()
{
	PrimaryActorTick.bCanEverTick = false;

	// 기본 루트 생성
	m_Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(m_Root);

	// 스폰 범위 지정할 박스 생성
	m_SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	m_SpawnBox->SetupAttachment(m_Root);

	// 기본 박스크기 설정
	// BP에서 조절 가능
	m_SpawnBox->SetBoxExtent(FVector(500.f, 500.f, 200.f));

	// 박스는 충돌 판정없음
	// 스폰 영역 계산 용도
	m_SpawnBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	m_SpawnBox->SetGenerateOverlapEvents(false);

	m_SpawnBox->SetHiddenInGame(true);
}

void AC_SpawnArea::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
		return;

	TryRegisterPointTowerManager();
}

bool AC_SpawnArea::IsZombieTypeAllowed(EZombieType _ZombieType) const
{
	// 비활성화 중인 Spawn
	if (!m_bEnabled)
		return false;

	// 허용 목록에 해당 좀비타입이 포함되어있는지 확인
	return m_AllowedZombieType.Contains(_ZombieType);
}

bool AC_SpawnArea::FindValidSpawnTransform(EZombieType _ZombieType, float _CapsuleRadius, float _CapsuleHalfHeight, FTransform& _OutTransform) const
{

	// 실패했을 때 이전 값이 남지 않도록 초기화
	_OutTransform = FTransform::Identity;

	// 현재 사용 불가능한 영역
	if (!m_bEnabled)
		return false;

	// 해당 영역에서 허용하지 않는 좀비 타입
	if (!IsZombieTypeAllowed(_ZombieType))
		return false;

	// 잘못된 캡슐 크기 방어
	if (_CapsuleRadius <= 0.f || _CapsuleHalfHeight <= 0.f)
		return false;

	UWorld* World = GetWorld();
	if (!IsValid(World))
		return false;

	// 현재 월드 NavigationSystem 가져오기
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
	if (!IsValid(NavSystem))
		return false;

	//최대 시도 횟수만큼 안전한 위치 탐색
	for (int32 Attempt = 0; Attempt < MaxSpawnAttempts; ++Attempt)
	{
		// 박스 안에서 무작위 후보위치 생성
		const FVector RandomPoint = GetRandomPointInSpawnBox();

		// 후보 위치를 가까운 NavMesh 위로 보정
		FNavLocation NavLocation;

		const bool bProjectedToNav = NavSystem->ProjectPointToNavigation(RandomPoint, NavLocation, m_NavProjectionExtent);

		if (!bProjectedToNav)
			continue;

		// NavMesh 보정 후 위치가 박스 밖으로 나갔는지 확인
		if (!IsPointInsideSpawnBoxXY(NavLocation.Location))
			continue;

		// NavMesh 위치는 바닥 기준으로 
		// 캐릭터 Actor 위치는 캡슐 중심이라
		// HalfHeight 만큼 위로 올려줌
		const FVector CapsuleCenter = NavLocation.Location + FVector::UpVector * (_CapsuleHalfHeight + m_GroundOffset);

		// 캡슐 크기로 장애물 겹침 검사
		if (IsSpawnLocationBlocked(CapsuleCenter, _CapsuleRadius, _CapsuleHalfHeight))
			continue;

		// 스폰 방향은 무작위 Yaw 사용
		const FRotator SpawnRotation(0.f, FMath::FRandRange(0.f, 360.f), 0.f);

		// 모든 검사를 통과한 최종 Transform 반환
		_OutTransform = FTransform(SpawnRotation, CapsuleCenter);

		return true;
	}

	// 최대 횟수까지 유효 위치를 찾지 못함
	return false;
}

FVector AC_SpawnArea::GetRandomPointInSpawnBox() const
{
	if (!IsValid(m_SpawnBox))
		return GetActorLocation();

	// 박스의 Scale까지 반영된 실제 크기
	const FVector ScaledExtent = m_SpawnBox->GetScaledBoxExtent();

	// 박스 중심 기준 무작위 위치
	const FVector LocalRandomPoint(FMath::FRandRange(-ScaledExtent.X, ScaledExtent.X),
		FMath::FRandRange(-ScaledExtent.Y, ScaledExtent.Y), 0);

	// TransformPositionNoScale 을 사용해 박스의 위치와 회전이 반영되게
	// 로컬 위치를 월드 위치로 변환
	return m_SpawnBox->GetComponentTransform().TransformPositionNoScale(LocalRandomPoint);
}

bool AC_SpawnArea::IsPointInsideSpawnBoxXY(const FVector& _WorldLocation) const
{
	if(!IsValid(m_SpawnBox))
		return false;

	// 월드위치를 SpawnBox 로컬좌표로 변환
	const FVector LocalLocation = m_SpawnBox->GetComponentTransform().InverseTransformPosition(_WorldLocation);

	// Scale이 적용되기 전 박스의 로컬 크기
	const FVector UnScaleExtent = m_SpawnBox->GetUnscaledBoxExtent();

	// XY가 박스 범위 내부인지 확인
	return FMath::Abs(LocalLocation.X) <= UnScaleExtent.X && FMath::Abs(LocalLocation.Y) <= UnScaleExtent.Y;
}

bool AC_SpawnArea::IsSpawnLocationBlocked(const FVector& _CapsuleCenter, float _CapsuleRadius, float _CapsuleHalfHeight) const
{
	const UWorld* World = GetWorld();

	if (!IsValid(World))
		return true;

	// 좀비와 같은 크기의 캡슐 충돌 형태 생성
	const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(_CapsuleRadius, _CapsuleHalfHeight);

	DrawDebugCapsule(GetWorld(), _CapsuleCenter, _CapsuleHalfHeight, _CapsuleRadius, FQuat::Identity, FColor::Red, true);
	
	FCollisionQueryParams QueryParams;

	// SpawnArea 자기 자신은 검사 제외
	QueryParams.AddIgnoredActor(this);

	// 해당 위치에 Pawn 캡슐을 놨을때
	// Blocking 충돌이 발생하는지 검사
	return World->OverlapBlockingTestByChannel(_CapsuleCenter, FQuat::Identity, ECC_Pawn, CapsuleShape, QueryParams);
}

void AC_SpawnArea::TryRegisterPointTowerManager()
{
	// 현재 게임모드 가져오기
	AC_GameMode_GameLv* GameMode = GetWorld()->GetAuthGameMode<AC_GameMode_GameLv>();

	if (!IsValid(GameMode))
		return;

	// 게임모드가 가지고 있는 PointTowerManager 가져오기
	UC_PointTowerManager* PointTowerManager = GameMode->GetPointTowerManager();

	if (!IsValid(PointTowerManager))
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AC_SpawnArea::TryRegisterPointTowerManager);

		return;
	}

	// 자기 자신을 Sequence에 맞게 등록
	if (!PointTowerManager->RegisterSpawnArea(this))
	{
		UC_Util::Print("From AC_SpawnArea::BeginPlay : RegisterSpawnArea Failed", FColor::Red, 10.f);
	}
}

