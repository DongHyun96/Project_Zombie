// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Zombie.h"


#include "../../../GlobalEnum.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/PointTower/C_PointTower.h"
#include "Net/UnrealNetwork.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ShapeComponent.h"
#include "Controller/C_ZombieController.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"

AC_Zombie::AC_Zombie()
	: m_ZombieType(EZombieType::NormalZombie)
{
	m_TeamId = static_cast<uint8>(ETeamType::Enemy);
}

AC_Zombie::AC_Zombie(EZombieType _ZombieType)
	: m_ZombieType(_ZombieType)
{
	m_TeamId = static_cast<uint8>(ETeamType::Enemy);
}

void AC_Zombie::BeginPlay()
{
	Super::BeginPlay();
	
	// 등록된 모든 NormalAttackCollider의 ComponentBeginOverlap 이벤트 바인딩 및 첫 시작 시, 비활성화 처리
	for (UShapeComponent* NormalAttackCollider : m_NormalAttackColliders)
	{
		// 서버 쪽에서만 실질적인 피격 판정 및 피격 처리가 들어갈 것임
		if (HasAuthority())
			NormalAttackCollider->OnComponentBeginOverlap.AddDynamic(this, &AC_Zombie::OnNormalAttackColliderBeginOverlap);
		
		NormalAttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AC_Zombie::ANS_OnNormalAttackStart()
{
	// 오로지 서버 쪽에서 피격 판정 및 데미지 주기 처리
	if (!HasAuthority()) return;

	// 피격판정 시작 전, Set 비우기
	m_SetNormalAttackColliderEntered.Empty();
	
	// 피격 검사 활성화
	for (UShapeComponent* AttackCollider : m_NormalAttackColliders)
		AttackCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AC_Zombie::ANS_OnNormalAttackEnd()
{
	if (!HasAuthority()) return;

	m_SetNormalAttackColliderEntered.Empty();

	for (UShapeComponent* AttackCollider : m_NormalAttackColliders)
		AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AC_Zombie::OnNormalAttackColliderBeginOverlap
(
	UPrimitiveComponent* OverlappedComponent,
	AActor*				 OtherActor,
	UPrimitiveComponent* OtherComp,
	int32				 OtherBodyIndex,
	bool				 bFromSweep,
	const FHitResult&	 SweepResult
)
{
	// Client 쪽은 Event 바인딩 처리 자체를 안해서 검사하지 않아도 됨
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor);
	AC_PointTower* PointTower = Cast<AC_PointTower>(OtherActor);
	if (!Player && !PointTower) return; // PointTower나 Player가 아닌 경우
	
	// 이미 이번 휘두르기에 피격처리가 한 번 들어감
	if (m_SetNormalAttackColliderEntered.Contains(Player)) return;
	
	m_SetNormalAttackColliderEntered.Add(Player);

	// 현재 Skill의 피격량을 구해와서, 대상 Target에게 ApplyDamage 처리
	UGameplayStatics::ApplyDamage
	(
		OtherActor,
		m_SkillCom->GetCurSkillDamage(),
		GetController(),
		this,
		nullptr
	);
}

void AC_Zombie::ApplyPoolActiveState()
{
	if (m_bPoolActive)
	{
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		SetActorTickEnabled(true);

		if (UCharacterMovementComponent* MoveCom = GetCharacterMovement())
		{
			MoveCom->StopMovementImmediately();
			MoveCom->SetMovementMode(EMovementMode::MOVE_Walking);
		}

		return;
	}

	// 풀 반환 시 남아있는 몽타주 정지
	StopAnimMontage();

	// 이동 정지
	if (UCharacterMovementComponent* MoveCom = GetCharacterMovement())
	{
		MoveCom->StopMovementImmediately();
		MoveCom->DisableMovement();
	}

	// 충돌 비활성화
	SetActorEnableCollision(false);

	// 화면 숨김처리
	SetActorHiddenInGame(true);

	// tick 비활성화
	SetActorTickEnabled(false);
	
	// BrainComponent 비활성화
	if (UBrainComponent* Brain = m_ZombieController->GetBrainComponent())
		Brain->StopLogic(TEXT("FirstInitPooling"));
}

void AC_Zombie::OnRep_PoolActive()
{
	// 클라이언트도 서버랑 같은 상태 적용
	ApplyPoolActiveState();
}

bool AC_Zombie::DeactivateForPool()
{
	// 풀 상태 변경은 서버에서만 처리
	if (!HasAuthority())
		return false;

	// 중복 처리 방지
	if (!m_bPoolActive)
		return false;

	m_bPoolActive = false;

	// 서버에 바로 비활성 상태 적용
	ApplyPoolActiveState();

	// 클라에 최대한 빨리 전달
	ForceNetUpdate();

	return true;
}

bool AC_Zombie::ActivateFromPool(const FTransform& _SpawnTransform)
{
	// 풀 활성 상태는 서버에서만 처리
	if (!HasAuthority())
		return false;

	// 이미 활성화 되어잇는 좀비는 중복처리 x
	if (m_bPoolActive)
		return false;

	// 스폰 위치 먼저 설정
	// 숨김 상태가 풀리기 전에 먼저 옮겨두려고 미리 설정
	SetActorTransform(_SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);

	// 좀비 공통 상태 초기화
	ResetEnemyForPoolSpawn();

	// 풀 활성 상태로 변경
	m_bPoolActive = true;

	// 서버에 활성 상태 적용
	ApplyPoolActiveState();

	// 죽은 이후 StopLogic으로 정지된 BT 다시 시작
	if (AAIController* pController = Cast<AAIController>(GetController()))
	{
		// 이전 이동 요청 제거
		pController->StopMovement();

		if (UBrainComponent* Brain = pController->GetBrainComponent())
		{
			Brain->RestartLogic();
		}
	}

	// 클라에 상태와 위치를 빠르게 전달
	ForceNetUpdate();

	return true;
}

void AC_Zombie::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 풀 활성 여부를 클라에 복제
	DOREPLIFETIME(AC_Zombie, m_bPoolActive);
}

void AC_Zombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
