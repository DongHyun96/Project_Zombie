// Fill out your copyright notice in the Description page of Project Settings.


#include "C_TankZombie.h"

#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "AIController.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "Actor/Character/Player/C_BasicPlayer.h"

#include "Kismet/GameplayStatics.h"


AC_TankZombie::AC_TankZombie()
{
	PrimaryActorTick.bCanEverTick = true;

	m_bCharging = false;
	m_Skill = nullptr;
	m_ChargeSpeed = 0.f;
	m_ChargeTarget = nullptr;

	m_ChargeCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ChargeCollision"));

	m_ChargeCollision->SetupAttachment(GetRootComponent());

	// 박스 컴포넌트의 크기와 위치는 블루프린트에서 조절

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
}

void AC_TankZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 실제 충돌과 판정은 서버가 담당
	if (!HasAuthority())
		return;

	if (!m_bCharging)
		return;

	UpdateCharge();
}

void AC_TankZombie::UpdateCharge()
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

	// 시작할 때 저장한 방향으로 직선 이동
	MoveCom->Velocity = m_ChargeDirection * m_ChargeSpeed;

	// 시작 위치에서 현재 위치까지의 수평거리
	const float TraveledDistance = FVector::Dist2D(m_ChargeStartLocation, GetActorLocation());

	// SkillData의 Range를 최대 돌진 거리로 사용
	if (TraveledDistance >= m_Skill->Range)
	{
		StopCharge();
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

bool AC_TankZombie::PrepareCharge(AActor* _Target, UC_EnemySkillData* _Data)
{
	if (!HasAuthority())
		return false;

	if (m_bCharging)
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

	if (m_bCharging)
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
	m_ChargeSpeed = MoveCom->MaxWalkSpeed * FMath::Max(m_Skill->MoveSpeedScale, 1.f);

	m_bCharging = true;

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
	m_ChargeTarget = nullptr;
	m_Skill = nullptr;
}
