// Fill out your copyright notice in the Description page of Project Settings.


#include "C_TankZombie.h"

#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "AIController.h"

#include "Animation/AnimInstance.h"

#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "Actor/Character/Player/C_BasicPlayer.h"

#include "Engine/World.h"
#include "Engine/OverlapResult.h"

#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "DrawDebugHelpers.h"

#include "Utility/C_Util.h"

#include "Kismet/GameplayStatics.h"

AC_TankZombie::AC_TankZombie()
	: Super(EZombieType::TankZombie)
{
	PrimaryActorTick.bCanEverTick = true;

	// 돌진 변수 초기화
	m_bCharging = false;
	m_Skill = nullptr;
	m_ChargeSpeed = 0.f;
	m_ChargeTarget = nullptr;

	m_PawnCollision = ECR_Block;
	m_ChargeElapsedTime = 0.f;
	m_ChargeMaxTime = 5.f;

	m_ChargeDirection = FVector::ZeroVector;
	m_ChargeStartLocation = FVector::ZeroVector;
	
	// End 이동
	m_bEndMoving = false;
	m_EndMoveDirection = FVector::ZeroVector;
	m_EndMoveSpeed = 600.f;
	m_EndMoveUpPower = 400.f;
	m_EndMoveMaxTime = 3.f;
	m_EndMoveElapsedTime = 0.f;

	// 착지 시 충격파 
	m_LandingShockRadius = 500.f;
	m_LandingShockUpPower = 650.f;
	m_LandingShockOutPower = 200.f;

	m_ChargeCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ChargeCollision"));

	m_ChargeCollision->SetupAttachment(GetRootComponent());

	// 박스 컴포넌트의 크기와 위치는 블루프린트에서 조절

	// 박스컴포넌트 설정
	m_ChargeCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	m_ChargeCollision->SetGenerateOverlapEvents(true);
	m_ChargeCollision->SetCollisionObjectType(ECC_WorldDynamic);
	m_ChargeCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	m_ChargeCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

}

void AC_TankZombie::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(m_ChargeCollision))
	{
		m_ChargeCollision->OnComponentBeginOverlap.AddDynamic(this, &AC_TankZombie::OnChargeBeginOverlap);
	}

	if (IsValid(GetCapsuleComponent()))
	{
		GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AC_TankZombie::OnChargeCapsuleHit);
	}
}

void AC_TankZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 실제 충돌과 판정은 서버가 담당
	if (!HasAuthority())
		return;

	// 돌진 이동
	if (m_bCharging)
	{
		UpdateCharge(DeltaTime);
	}

	// 점프 착지 이동
	if (m_bEndMoving)
	{
		UpdateEndMove(DeltaTime);
	}
}

void AC_TankZombie::UpdateCharge(float DeltaTime)
{
	if (!m_bCharging)
		return;

	if (!IsValid(m_Skill))
	{
		StopCharge();
		return;
	}

	UCharacterMovementComponent* MoveCom = GetCharacterMovement();

	if (!IsValid(MoveCom))
	{
		StopCharge();
		return;
	}

	// 돌진 지속시간 증가
	m_ChargeElapsedTime += DeltaTime;

	// 거리 계산이 정상적으로 증가하지 않는 상황을 대비한 안전장치
	// 벽이나 다른 충돌도 막혀도 Run 반복하지않음
	if (m_ChargeElapsedTime >= m_ChargeMaxTime)
	{
		StopCharge();
		return;
	}

	// 시작할 때 저장한 방향으로 직선 이동
	MoveCom->Velocity = m_ChargeDirection * m_ChargeSpeed;

	// 시작 위치에서 현재 위치까지의 수평거리
	const float TraveledDistance = FVector::Dist2D(m_ChargeStartLocation, GetActorLocation());

	// SkillData의 Range를 최대 돌진 거리로 사용
	if (TraveledDistance >= m_Skill->Range)
	{
		StopCharge();
		return;
	}
}

void AC_TankZombie::OnChargeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
		return;

	if (!m_bCharging || !IsValid(m_Skill))
		return;

	if (!IsValid(OtherActor))
		return;

	// 본인 무시
	if (OtherActor == this)
		return;

	// 이미 충돌한 대상이면 무시
	if (m_ChargeHitTarget.Contains(OtherActor))
		return;

	// 충돌 대상이 플레이어라면
	// 데미지 + 넉백
	if (AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor))
	{
		m_ChargeHitTarget.Add(Player);
		
		HandlePlayerHit(Player);

		// 플레이어가 맞았으면 돌진 종료
		StopCharge();
		return;
	}

	// 충돌 대상이 좀비라면
	// 데미지X 넉백만
	if (AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(OtherActor))
	{
		m_ChargeHitTarget.Add(Enemy);
		HandleEnemyHit(Enemy);
		return;
	}

}

void AC_TankZombie::OnChargeCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//UC_Util::Print("Wall Hit");

	if (!HasAuthority())
		return;

	// 돌진 중이 아니면 일반적인 충돌이라 무시
	if (!m_bCharging)
		return;

	if (!IsValid(OtherActor) || OtherActor == this)
		return;

	if (!IsValid(OtherComp))
		return;

	const ECollisionChannel ObjectType = OtherComp->GetCollisionObjectType();

	// 맵의 벽, 건물, 장애물 등에 충돌한 경우
	const bool bIsWorldObj = ObjectType == ECC_WorldDynamic || ObjectType == ECC_WorldStatic;
	if (!bIsWorldObj)
		return;

	// 바닥도 worldStatic 이기 때문에 정면에 있는 벽만 방향 검사하도록

	// 충돌 표면의 수평 방향
	FVector Wall = Hit.ImpactNormal;
	Wall.Z = 0.f;
	Wall = Wall.GetSafeNormal();

	// 바다이나 경사면같은 수평 노멀을 구할 수 없는 충돌은 무시
	if (Wall.IsNearlyZero())
		return;

	// 진행 방향과 벽을 향하는 방향이 얼마나 일치하는지 확인
	const float FrontDot = FVector::DotProduct(m_ChargeDirection, -Wall);

	// 탱크의 진행 방향 앞쪽에 있는 벽일때만 종료
	if (FrontDot < 0.5f)
		return;

	StopCharge();
}

bool AC_TankZombie::PrepareCharge(AActor* _Target, UC_EnemySkillData* _Data)
{
	if (!HasAuthority())
		return false;

	if (m_bCharging || m_bEndMoving)
		return false;

	if (!IsValid(_Target) || !IsValid(_Data))
		return false;


	// 타겟 방향 바라보게 하기
	FVector Direction = _Target->GetActorLocation() - GetActorLocation();

	Direction.Z = 0.f;
	Direction = Direction.GetSafeNormal();

	if (Direction.IsNearlyZero())
		return false;

	m_ChargeTarget = _Target;
	m_Skill = _Data;

	// Roar 동안 BT의 Move To가 움직이지 않게 정지
	if (AAIController* pController = Cast<AAIController>(GetController()))
	{
		pController->StopMovement();
	}

	SetActorRotation(Direction.Rotation());

	// SkillData의 FireSound를 돌진 시작 시 포효음으로 사용
	//if (IsValid(m_Skill->FireSound))
	//{
	//	UGameplayStatics::PlaySoundAtLocation(this, m_Skill->FireSound, GetActorLocation());
	//}

	return true;
}

void AC_TankZombie::LandingImpact()
{
	if (!HasAuthority())
		return;

	// 중복 착지 처리 방지
	if (!m_bEndMoving)
		return;

	// 착지 후 End 이동정지
	StopEndMove();

	// 주변 플레이어와 좀비띄우기
	ApplyLandingShock();
}

void AC_TankZombie::BeginPreparedCharge()
{
	if (!HasAuthority())
		return;

	if (m_bCharging)
		return;

	if (!IsValid(m_ChargeTarget) || !IsValid(m_Skill))
		return;

	StartCharge(m_ChargeTarget, m_Skill);
}

void AC_TankZombie::StartCharge(AActor* _Target, UC_EnemySkillData* _SkillData)
{
	if (!HasAuthority())
		return;

	if (m_bCharging || m_bEndMoving)
		return;

	if (!IsValid(_Target) || !IsValid(_SkillData))
		return;

	UCharacterMovementComponent* MoveCom = GetCharacterMovement();

	if (!IsValid(MoveCom))
		return;

	FVector Direction = _Target->GetActorLocation() - GetActorLocation();

	// 수평방향으로만 돌진
	Direction.Z = 0.f;
	Direction = Direction.GetSafeNormal();

	if (Direction.IsNearlyZero())
		return;

	// 돌진에 사용되는 SkillData 저장
	//m_ChargeTarget = _Target;
	//m_Skill = _SkillData;

	m_ChargeDirection = Direction;
	m_ChargeStartLocation = GetActorLocation();

	// 돌진속도 = 기본 이동속도 * 스킬 이동속도 배율
	m_Skill = _SkillData;
	m_ChargeTarget = _Target;

	m_ChargeSpeed = MoveCom->MaxWalkSpeed * FMath::Max(_SkillData->MoveSpeedScale, 1.f);

	m_bCharging = true;

	// 돌진 지속 시간 초기화
	m_ChargeElapsedTime = 0.f;

	// 돌진 중에 플레이어 캡슐에 막히지 않도록 설정
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		// 돌진 전 충돌 설정 저장
		m_PawnCollision = Capsule->GetCollisionResponseToChannel(ECC_Pawn);

		// 실제 충돌은 박스가 담당
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}

	// 이전 돌진에서 기록한 충돌 대상을 초기화
	m_ChargeHitTarget.Reset();

	// BT의 MoveTo가 돌진을 방해하지 않도록 정지
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}

	// 돌진 시작할 때 목표 방향으로 회전
	SetActorRotation(m_ChargeDirection.Rotation());

	// 돌진 충돌박스 활성화
	if (IsValid(m_ChargeCollision))
	{
		m_ChargeCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

}

void AC_TankZombie::StopCharge()
{
	if (!HasAuthority())
		return;

	if (!m_bCharging)
		return;

	m_bCharging = false;

	// 돌진이 끝난 후 탱크캡슐 pawn 충돌설정 복구
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, m_PawnCollision);
	}

	// 충돌박스 비활성화
	if (IsValid(m_ChargeCollision))
	{
		m_ChargeCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 낙하 중일 수 있으므로 Z속도는 유지하고 수평속도만 제거
	if (UCharacterMovementComponent* MoveCom = GetCharacterMovement())
	{
		FVector CurrentVelocity = MoveCom->Velocity;

		CurrentVelocity.X = 0.f;
		CurrentVelocity.Y = 0.f;

		MoveCom->Velocity = CurrentVelocity;
	}

	// End 구간 실제 점프이동 시작
	StartEndMove();

	// Run 몽타주 섹션 루프에서 End 섹션으로 이동
	if (IsValid(m_Skill) && IsValid(m_Skill->Montage))
	{
		if (USkeletalMeshComponent* pMesh = GetMesh())
		{
			if (UAnimInstance* AnimInst = pMesh->GetAnimInstance())
			{
				AnimInst->Montage_JumpToSection(TEXT("End"), m_Skill->Montage);
			}
		}
	}

	// 위치, 속도, 방향값 초기화
	m_ChargeDirection = FVector::ZeroVector;
	m_ChargeStartLocation = FVector::ZeroVector;
	m_ChargeSpeed = 0.f;
	m_ChargeElapsedTime = 0.f;

	// 등록한 충돌타겟들 초기화
	m_ChargeHitTarget.Reset();
	m_ChargeTarget = nullptr;
}


void AC_TankZombie::HandlePlayerHit(AC_BasicPlayer* _Player)
{
	if (!IsValid(_Player) || !IsValid(m_Skill))
		return;

	// SkillData의 넉백수치 사용
	const FVector KnockbackVelocity = m_ChargeDirection * m_Skill->KnockbackPower +
										FVector::UpVector * m_Skill->KnockbackUpPower; 

	// SkillData의 Damage 사용
	UGameplayStatics::ApplyDamage(_Player, m_Skill->Damage, GetController(), this, UDamageType::StaticClass());

	_Player->LaunchCharacter(KnockbackVelocity, true, true);

	// SkillData의 HitSound를 충돌음으로 사용
	if (IsValid(m_Skill->HitSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_Skill->HitSound, _Player->GetActorLocation());
	}
}

void AC_TankZombie::HandleEnemyHit(AC_BasicEnemy* _Enemy)
{
	if (!IsValid(_Enemy) || !IsValid(m_Skill))
		return;

	// 탱크 중심에서 충돌한 좀비 쪽으로 향하는 방향
	FVector KnockbackDir = _Enemy->GetActorLocation() - GetActorLocation();

	// 수평으로 밀어낼 것이기때문에 높이 차이는 제거
	KnockbackDir.Z = 0.f;
	KnockbackDir = KnockbackDir.GetSafeNormal();

	// 두 액터가 거의 같은 위치라 방향을 구하지 못한다면
	// 탱크의 진행 방향을 대신 사용
	if (KnockbackDir.IsNearlyZero())
	{
		KnockbackDir = m_ChargeDirection;
	}

	// 바깥 방향으로 미는 힘 + 약간 위로 띄우는 힘
	const FVector KnockbackVelocity = KnockbackDir * m_Skill->FriendlyKnockbackPower +
										FVector::UpVector * m_Skill->KnockbackUpPower;

	// Enemy에게는 데미지 없이 넉백만 적용
	_Enemy->LaunchCharacter(KnockbackVelocity, true, true);

}

void AC_TankZombie::CancelPrepareCharge()
{
	if (!HasAuthority())
		return;

	// 실제 돌진이 시작됏다면 취소하지 않음
	if (m_bCharging)
		return;

	m_ChargeTarget = nullptr;
	m_Skill = nullptr;
}

void AC_TankZombie::FinishChargeSkill()
{
	if (HasAuthority())
	{
		m_bCharging = false;
		StopEndMove();

		// 캡슐 pawn 충돌 설정 복구
		if (UCapsuleComponent* Capsule = GetCapsuleComponent())
		{
			Capsule->SetCollisionResponseToChannel(ECC_Pawn, m_PawnCollision);
		}

		if (IsValid(m_ChargeCollision))
		{
			m_ChargeCollision->SetCollisionEnabled(
				ECollisionEnabled::NoCollision);
		}
	}

	m_ChargeDirection = FVector::ZeroVector;
	m_ChargeStartLocation = FVector::ZeroVector;
	m_ChargeSpeed = 0.f;
	m_ChargeElapsedTime = 0.f;

	m_EndMoveDirection = FVector::ZeroVector;
	m_EndMoveElapsedTime = 0.f;

	m_ChargeTarget = nullptr;
	m_Skill = nullptr;

	m_ChargeHitTarget.Reset();
}


void AC_TankZombie::StartEndMove()
{
	if (!HasAuthority())
		return;

	if (m_bEndMoving)
		return;

	// 돌진중 사용하던 방향을 End 이동방향으로 저장
	m_EndMoveDirection = m_ChargeDirection;

	m_EndMoveDirection.Z = 0.f;
	m_EndMoveDirection = m_EndMoveDirection.GetSafeNormal();

	if (m_EndMoveDirection.IsNearlyZero())
		return;

	m_bEndMoving = true;
	m_EndMoveElapsedTime = 0.f;

	// 탱크의 캡슐을 앞 + 위로 발사
	const FVector LaunchVelocity = m_EndMoveDirection * m_EndMoveSpeed + FVector::UpVector * m_EndMoveUpPower;

	LaunchCharacter(LaunchVelocity, true, true);
}

void AC_TankZombie::UpdateEndMove(float DeltaTime)
{
	if (!m_bEndMoving)
		return;

	m_EndMoveElapsedTime += DeltaTime;

	// 착지 노티파이가 호출되지 않았을 때를 위한 안전장치
	if (m_EndMoveElapsedTime >= m_EndMoveMaxTime)
	{
		StopEndMove();
	}

}

void AC_TankZombie::StopEndMove()
{
	m_bEndMoving = false;
	m_EndMoveElapsedTime = 0.f;

	//if (UCharacterMovementComponent* MoveCom = GetCharacterMovement())
	//{
	//	FVector CurVeolocity = MoveCom->Velocity;
	//
	//	// 수평이동만 정지
	//	CurVeolocity.X = 0.f;
	//	CurVeolocity.Y = 0.f;
	//
	//	// Z속도 유지
	//	MoveCom->Velocity = CurVeolocity;
	//}

	m_EndMoveDirection = FVector::ZeroVector;
}

void AC_TankZombie::ApplyLandingShock()
{
	if (!HasAuthority())
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	const FVector ImpactCenter = GetActorLocation();

	TArray<FOverlapResult> OverlapResult;

	// pawn만 검색
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	// 본인은 판정 제외
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(m_LandingShockRadius);

	const bool bOverlap = World->OverlapMultiByObjectType(	
															OverlapResult,
															ImpactCenter,
															FQuat::Identity,
															ObjectQueryParams,
															SphereShape,
															QueryParams);

	// 테스트 디버그스피어
	DrawDebugSphere(World, ImpactCenter, m_LandingShockRadius, 24, FColor::Red, false, 2.f);

	if (!bOverlap)
		return;

	// 한 액터에 여러 컴포넌트가 겹쳐도 한 번만 처리
	TSet<TWeakObjectPtr<AActor>> AppliedTargets;
	
	for (const FOverlapResult& Result : OverlapResult)
	{
		AActor* Target = Result.GetActor();

		if (!IsValid(Target))
			continue;

		if (Target == this)
			continue;

		// 이미 처리한 액터면 무시
		if (AppliedTargets.Contains(Target))
			continue;

		ACharacter* TargetCharacter = Cast<ACharacter>(Target);
		if (!IsValid(TargetCharacter))
			continue;

		// 플레이어나 좀비만 처리
		const bool bIsPlayer = Target->IsA<AC_BasicPlayer>();
		const bool bIsEnemy = Target->IsA<AC_BasicEnemy>();

		if (!bIsPlayer && !bIsEnemy)
			continue;

		AppliedTargets.Add(Target);

		// 착지 중심에서 대상 방향으로 바깥 방향 계산
		FVector OutDirection = Target->GetActorLocation() - ImpactCenter;

		OutDirection.Z = 0.f;
		OutDirection = OutDirection.GetSafeNormal();

		// 위치가 완전히 겹쳐 방향이 없는 경우
		if (OutDirection.IsNearlyZero())
		{
			OutDirection = GetActorForwardVector();
			OutDirection.Z = 0.f;
			OutDirection.Normalize();
		}

		const FVector LaunchVelocity = OutDirection * m_LandingShockOutPower + FVector::UpVector * m_LandingShockUpPower;

		TargetCharacter->LaunchCharacter(LaunchVelocity, true, true);
	}
}

void AC_TankZombie::CancelChargeForDead()
{
	// 서버에서만 돌진 상태 정리
	if (!HasAuthority())
		return;

	// 돌진 및 End 이동 상태 정리
	m_bCharging = false;
	StopEndMove();

	// 돌진 판정용 충돌박스 비활성화
	if (IsValid(m_ChargeCollision))
	{
		m_ChargeCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 캡슐 pawn 충돌 설정 복구
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, m_PawnCollision);
	}

	// 현재 이동속도 제거
	if (UCharacterMovementComponent* MoveCom = GetCharacterMovement())
	{
		MoveCom->StopMovementImmediately();
	}

	// 돌진 관련 설정 초기화
	m_ChargeDirection = FVector::ZeroVector;
	m_ChargeStartLocation = FVector::ZeroVector;
	m_ChargeSpeed = 0.f;
	m_ChargeElapsedTime = 0.f;
	
	m_ChargeTarget = nullptr;
	m_Skill = nullptr;
	m_ChargeHitTarget.Reset();
}

void AC_TankZombie::OnDead(AC_BasicCharacter* _DeadCharacter)
{
	// 본인이 죽었을때만 탱크 돌진 상태 정리
	if (_DeadCharacter == this)
	{
		CancelChargeForDead();
	}

	// 공통 사망 처리
	Super::OnDead(_DeadCharacter);
}
