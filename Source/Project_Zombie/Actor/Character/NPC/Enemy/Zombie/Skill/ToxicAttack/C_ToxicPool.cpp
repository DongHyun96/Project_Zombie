// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ToxicPool.h"

#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"

#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"


AC_ToxicPool::AC_ToxicPool()
{
	PrimaryActorTick.bCanEverTick = false;

	// 1. 충돌 컴포넌트
	m_Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	RootComponent = m_Sphere;
	m_Sphere->InitSphereRadius(10.f);
	// SetCollisionProfileName(TEXT("Projectile"));

	// 2. NiagaraComponent
	m_NiagaraCom = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	m_NiagaraCom->SetupAttachment(m_Sphere); // 자식으로 설정

	m_Sphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&AC_ToxicPool::OnBeginOverlap);

	m_Sphere->OnComponentEndOverlap.AddDynamic(
		this,
		&AC_ToxicPool::EndOverlap);
}

void AC_ToxicPool::InitPool(AC_BasicEnemy* _SkillUser, UC_EnemySkillData* _Skill)
{
	m_SkillUser = _SkillUser;
	m_Skill = _Skill;
}


void AC_ToxicPool::BeginPlay()
{
	Super::BeginPlay();

	if (m_PoolEffect)
	{
		m_NiagaraCom->SetAsset(m_PoolEffect);
	}
	
	if (m_Skill)
	{
		SetLifeSpan(m_Skill->PoolLifetime);
	}
	else
	{
		SetLifeSpan(5.f);
	}
}

void AC_ToxicPool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AC_ToxicPool::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AC_ToxicPool::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}