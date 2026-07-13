// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "Kismet/GameplayStatics.h"

AC_EnemyProjectile::AC_EnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// 1. 충돌 컴포넌트
	m_Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	RootComponent = m_Sphere;
	m_Sphere->InitSphereRadius(10.f);
	// SetCollisionProfileName(TEXT("Projectile"));

	// 2. PMC(Projectile Movement Component)
	m_PMC = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("PMC"));
	m_PMC->InitialSpeed = 2000.f;
	m_PMC->MaxSpeed = 2000.f;
	m_PMC->bShouldBounce = false;
	m_PMC->Bounciness = 0.f;

	// 3. NiagaraComponent
	m_NiagaraCom = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	m_NiagaraCom->SetupAttachment(m_Sphere); // 자식으로 설정

	m_LifeTime = 5.f; // 기본 수명 설정(초기값 설정 없을 시 기본값)
}

void AC_EnemyProjectile::InitProjectile(AC_BasicEnemy* _SkillUser, UC_EnemySkillData* _Skill)
{
	m_SkillUser = _SkillUser;
	m_Skill = _Skill;

	if (_Skill)
	{
		m_LifeTime = _Skill->ProjectileLifetime;

		m_PMC->InitialSpeed = _Skill->ProjectileSpeed;
		m_PMC->MaxSpeed = _Skill->ProjectileSpeed;
	}

	SetLifeSpan(m_LifeTime);
}

void AC_EnemyProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 나이아가라 재생
	if (m_NiagaraCom && m_ProjectileEffect)
	{
		m_NiagaraCom->SetAsset(m_ProjectileEffect);
	}
}

void AC_EnemyProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AC_EnemyProjectile::OnHit()
{
	SpawnHitEffect();
	PlayHitSound();

	Destroy();
}


void AC_EnemyProjectile::SpawnHitEffect()
{
	if (m_HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), m_HitEffect, GetActorLocation(), GetActorRotation());
	}
}

void AC_EnemyProjectile::PlayHitSound()
{
	if (m_Skill && m_Skill->HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), m_Skill->HitSound, GetActorLocation());
	}
}
