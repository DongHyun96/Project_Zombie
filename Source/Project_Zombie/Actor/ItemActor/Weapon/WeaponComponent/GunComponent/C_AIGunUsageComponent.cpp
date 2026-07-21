// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AIGunUsageComponent.h"

#include "Actor/Character/C_BasicCharacter.h"
#include "Actor/Character/NPC/Enemy/Zombie/Controller/C_ZombieController.h"
#include "Actor/Character/NPC/Enemy/Zombie/CopZombie/C_CopZombie.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"


UC_AIGunUsageComponent::UC_AIGunUsageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UC_AIGunUsageComponent::BeginPlay()
{
	Super::BeginPlay();
	
	m_OwnerGun = Cast<AC_GunBase>(GetOwner());
	if (!m_OwnerGun) UC_Util::Print("From UC_AIGunUsageComponent::BeginPlay : This Component is for GunType Actor", FColor::Red, 10.f);
}

bool UC_AIGunUsageComponent::AIFire()
{
	// 사격 불가능한 상황
	if (--m_OwnerGun->m_CurrentAmmo <= 0)
	{
		m_OwnerGun->m_CurrentAmmo = 0;
		return false;
	}

	UC_Util::Print("Current Ammo : " + FString::FromInt(m_OwnerGun->m_CurrentAmmo), FColor::Green, 5.f);
	
	// 총기 자체의 발사 애니메이션 재생
	if (m_OwnerGun->m_WeaponMesh && m_OwnerGun->m_FireAnimation)
		m_OwnerGun->m_WeaponMesh->PlayAnimation(m_OwnerGun->m_FireAnimation, false);
	
	m_OwnerGun->SpawnShellEject();

	// 사격 방면 LineTrace Damage 처리
	AIProcessLineTraceDamage(m_OwnerGun->m_BaseDamage);
	
	// 사격 성공
	return true;
}

bool UC_AIGunUsageComponent::AttachToHand(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	m_WeaponCopZombieUser = Cast<AC_CopZombie>(_ParentMesh->GetOwner());
	if (!m_WeaponCopZombieUser) return false;

	const bool Attached = m_OwnerGun->AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		m_OwnerGun->s_HandSocketName
	);

	// 제대로 Attach되지 않은 상황 (여기 들어오면 안됨)
	if (!Attached)
	{
		UC_Util::Print("From UC_AIGunUsageComponent::AttachToHand : AttachToComponent failed!", FColor::Red, 10.f);
		return false;
	}
	
	// MaxAmmoCount로 탄창 초기화 처리
	// m_OwnerGun->m_CurrentAmmo = m_OwnerGun->m_MaxAmmo;
	m_OwnerGun->m_CurrentAmmo = 10; // TODO : 위의 코드로 수정할 것

	// 이미 사격중이었던 Weapon인 경우, Trigger 해제
	m_OwnerGun->ReleaseTrigger();
	m_OwnerGun->m_WeaponPlayerUser = nullptr; // -> Enemy가 이용을 끝낸 총기의 경우 누구누구의 총기 여기있다 표시를 하려면 해당 Owner필요함
	
	return true;
}

bool UC_AIGunUsageComponent::DetachFromHand()
{
	// 이 무기를 사용중인 CopZombie가 없을 때(또는 Valid하지 않은 경우)
	if (!m_WeaponCopZombieUser) return false;
	
	m_OwnerGun->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// RootComponent(MainCollider) 비활성화
	m_OwnerGun->m_Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// WeaponMesh의 Physics 활성화 처리 이전에, 이전 주인과의 충돌을 비활성화 처리
	m_OwnerGun->GetWeaponMesh()->IgnoreActorWhenMoving(m_WeaponCopZombieUser, true);
	
	// 지형지물 충돌 활성화 및 Physics 활성화 처리
	m_OwnerGun->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	m_OwnerGun->GetWeaponMesh()->SetSimulatePhysics(true);

	// 캐릭터가 바라보는 방향으로 살짝 위로 Drop 되는 느낌 처리를 줌
	
	const FVector ImpulseVector = m_WeaponCopZombieUser->GetActorForwardVector() * 350.f +
								  m_WeaponCopZombieUser->GetActorUpVector() * 200.f;
	
	m_OwnerGun->GetWeaponMesh()->AddImpulse(ImpulseVector, NAME_None, true);

	GetWorld()->GetTimerManager().SetTimer
	(
		m_GunMeshStoppedCheckTimer,
		this,
		&UC_AIGunUsageComponent::HandleGunMeshPhysicsStopped,
		0.5f,
		true
	);
	
	return true;
}

void UC_AIGunUsageComponent::AIProcessLineTraceDamage(float _DamageVal)
{
	if (!m_WeaponCopZombieUser)
	{
		UC_Util::Print("From UC_AIGunUsageComponent::AIProcessLineTraceDamage : AI Weapon user nullptr", FColor::Red, 10.f);
		return;
	}
	
	AActor* Target = m_WeaponCopZombieUser->GetZombieController()->GetCurrentBBTarget();
	
	// TODO : 무조건 Target을 맞추는 것이 아닌 오차를 좀 주긴 해야 함 (일단은 Target을 무조건 맞추는 처리로 함)
	// 사망 시에도 EndLocation Front 방면으로 줄것
	const FVector StartLocation = m_OwnerGun->GetWeaponMesh()->GetSocketLocation(TEXT("MuzzleFlash"));
	const FVector EndLocation   = Target ? Target->GetActorLocation()
										   : StartLocation + m_OwnerGun->GetWeaponMesh()->GetSocketRotation(TEXT("MuzzleFlash")).Vector() * 5000.0f;
		                              
	
	FHitResult HitResult{};
	FCollisionQueryParams QueryParams{};
	QueryParams.AddIgnoredActor(m_OwnerGun);
	QueryParams.AddIgnoredActor(m_WeaponCopZombieUser);
	
	bool bHasHit = GetWorld()->LineTraceSingleByChannel
	(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);
	const FVector ActualEndLocation = bHasHit ? HitResult.ImpactPoint : EndLocation;
	
	DrawDebugLine(GetWorld(), StartLocation, ActualEndLocation, FColor::Green, false, 5.f);

	if (bHasHit)
	{
		DrawDebugSphere(GetWorld(), ActualEndLocation, 7.f, 12, FColor::Red, true);

		// TODO : 거점사격에 대한 처리도 해주어야 함 (거점 퍼센티지 다운)
		if (AC_BasicCharacter* HitCharacter = Cast<AC_BasicCharacter>(HitResult.GetActor()))
			UGameplayStatics::ApplyDamage(HitCharacter, _DamageVal, m_WeaponCopZombieUser->GetController(), m_OwnerGun, nullptr);
	}
}

void UC_AIGunUsageComponent::HandleGunMeshPhysicsStopped()
{
	if (m_OwnerGun->GetWeaponMesh()->IsAnyRigidBodyAwake())
	{
		UC_Util::Print("RBODY AWAKE", FColor::MakeRandomColor(), 2.f);
		return;
	}
	
	GetWorld()->GetTimerManager().ClearTimer(m_GunMeshStoppedCheckTimer);
	
	m_OwnerGun->GetWeaponMesh()->IgnoreActorWhenMoving(m_WeaponCopZombieUser, false);
	m_OwnerGun->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	m_OwnerGun->GetWeaponMesh()->SetSimulatePhysics(false);

	// 파밍 처리 가능하게끔 MainCollider 활성화 (Overlap 검사)
	m_OwnerGun->m_Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// TODO : 무기 외곽에 Outline 활성화 및 WorldPingActor 스폰 처리
}

