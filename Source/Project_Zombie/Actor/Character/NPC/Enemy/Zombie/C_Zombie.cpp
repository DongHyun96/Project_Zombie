// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Zombie.h"


#include "../../../GlobalEnum.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ShapeComponent.h"
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
		if (IsLocallyControlled())
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

	UC_Util::Print("On ANSNormalAttack End", FColor::MakeRandomColor(), 20.f);
	
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
	
	// 이미 이번 휘두르기에 피격처리가 한 번 들어감
	if (m_SetNormalAttackColliderEntered.Contains(Player)) return;
	
	m_SetNormalAttackColliderEntered.Add(Player);

	// 현재 Skill의 피격량을 구해와서, 대상 Player에게 ApplyDamage 처리
	UGameplayStatics::ApplyDamage
	(
		Player,
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
		// 재스폰 구현할때 활성화 처리 추가하기
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		SetActorTickEnabled(true);

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
}

void AC_Zombie::OnRep_PoolActive()
{
	// 클라이언트도 서버랑 같은 상태 적용
	ApplyPoolActiveState();
}

void AC_Zombie::DeactivateForPool()
{
	// 풀 상태 변경은 서버에서만 처리
	if (!HasAuthority())
		return;

	// 중복 처리 방지
	if (!m_bPoolActive)
		return;

	m_bPoolActive = false;

	// 서버에 바로 비활성 상태 적용
	ApplyPoolActiveState();

	// 클라에 최대한 빨리 전달
	ForceNetUpdate();
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
