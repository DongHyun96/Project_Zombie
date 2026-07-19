// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "Kismet/GameplayStatics.h"

#include "Utility/C_Util.h"



AC_EnemyProjectile::AC_EnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// 1. 충돌 컴포넌트
	m_Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	RootComponent = m_Sphere;
	m_Sphere->InitSphereRadius(10.f);
	// SetCollisionProfileName(TEXT("Projectile"));

	m_Sphere->OnComponentHit.AddDynamic(this, &AC_EnemyProjectile::OnProjectileHit);

	// 2. PMC(Projectile Movement Component)
	m_PMC = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("PMC"));
	m_PMC->InitialSpeed = 2000.f;
	m_PMC->MaxSpeed = 2000.f;
	m_PMC->bShouldBounce = false;
	m_PMC->Bounciness = 0.f;

	m_PMC->UpdatedComponent = m_Sphere;

	// 3. NiagaraComponent
	m_NiagaraCom = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	m_NiagaraCom->SetupAttachment(m_Sphere); // 자식으로 설정

	m_LifeTime = 5.f; // 기본 수명 설정(초기값 설정 없을 시 기본값)
}

void AC_EnemyProjectile::InitProjectile(AC_BasicEnemy* _SkillUser, UC_EnemySkillData* _Skill)
{
	check(_Skill);
	check(_SkillUser);

	m_SkillUser = _SkillUser;
	m_Skill = _Skill;

	m_Sphere->IgnoreActorWhenMoving(m_SkillUser, true);

	m_LifeTime = m_Skill->ProjectileLifetime;

	m_PMC->InitialSpeed = _Skill->ProjectileSpeed;
	m_PMC->MaxSpeed = _Skill->ProjectileSpeed;
	m_PMC->Velocity = GetActorForwardVector() * _Skill->ProjectileSpeed;

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

void AC_EnemyProjectile::OnHit(AActor* _OtherActor, UPrimitiveComponent* _OtherCom, const FHitResult& _Hit)
{
	FVector HitLocation = GetActorLocation();

	if (_Hit.bBlockingHit)
	{
		HitLocation = _Hit.ImpactPoint;
	}

	SpawnHitEffect(HitLocation);
	PlayHitSound(HitLocation);

	Destroy();
}


void AC_EnemyProjectile::SpawnHitEffect(const FVector& _Location)
{
	if (!m_HitEffect)
		return;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), m_HitEffect, _Location, GetActorRotation());
}

void AC_EnemyProjectile::PlayHitSound(const FVector& _Location)
{
	if (!m_Skill || !m_Skill->HitSound)
		return;

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), m_Skill->HitSound, _Location);
}

void AC_EnemyProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == m_SkillUser)
		return;

	UC_Util::Print("Projectile Hit");

	OnHit(OtherActor, OtherComp, Hit);
}
