// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Zombie.h"


#include "../../../GlobalEnum.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Components/ShapeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"

AC_Zombie::AC_Zombie()
	: m_ZombieType(EZombieType::NormalZombie)
{
}

AC_Zombie::AC_Zombie(EZombieType _ZombieType)
	: m_ZombieType(_ZombieType)
{
}

void AC_Zombie::BeginPlay()
{
	Super::BeginPlay();
	
	// 팀 설정
	SetGenericTeamId(static_cast<uint8>(ETeamType::Enemy));

	UC_Util::Print("NormalAttackCollider Count : " + FString::FromInt(m_NormalAttackColliders.Num()), FColor::Red, 10.f);
	
	// 등록된 모든 NormalAttackCollider의 ComponentBeginOverlap 이벤트 바인딩 및 첫 시작 시, 비활성화 처리
	for (UShapeComponent* NormalAttackCollider : m_NormalAttackColliders)
	{
		// 서버 쪽에서만 실질적인 피격 판정 및 피격 처리가 들어갈 것임
		if (IsLocallyControlled())
		{
			NormalAttackCollider->OnComponentBeginOverlap.AddDynamic(this, &AC_Zombie::OnNormalAttackColliderBeginOverlap);
			UC_Util::Print("NormalAttackCollider BeginOverlap Event binded!", FColor::Red, 10.f);
		}
		
		NormalAttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AC_Zombie::ANS_OnNormalAttackStart()
{
	// 오로지 서버 쪽에서 피격 판정 및 데미지 주기 처리
	if (!HasAuthority()) return;

	UC_Util::Print("On ANS NormalAttackStart", FColor::MakeRandomColor(), 10.f);
	
	// 피격판정 시작 전, Set 비우기
	m_SetNormalAttackColliderEntered.Empty();
	
	// 피격 검사 활성화
	for (UShapeComponent* AttackCollider : m_NormalAttackColliders)
		AttackCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AC_Zombie::ANS_OnNormalAttackEnd()
{
	if (!HasAuthority()) return;

	UC_Util::Print("On ANS NormalAttackEnd", FColor::MakeRandomColor(), 10.f);
	
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
	
	UC_Util::Print("Overlapped with player", FColor::MakeRandomColor(), 10.f);

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

void AC_Zombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
