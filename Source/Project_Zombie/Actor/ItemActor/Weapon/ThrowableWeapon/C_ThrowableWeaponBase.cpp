// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ThrowableWeaponBase.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameMode/C_UIManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"

const FName AC_ThrowableWeaponBase::s_HolsterSocketName = TEXT("ThrowableHolsterSocket");

// Sets default values
AC_ThrowableWeaponBase::AC_ThrowableWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	m_ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	
	m_MainCollider = CreateDefaultSubobject<UCapsuleComponent>("Capsule");
	m_MainCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 직접 투척하기 이전까지는 Collision을 비활성화 처리해주어야 한다
	SetRootComponent(m_MainCollider);
	
	// TODO : PathSpline으로 예측 경로 그리기 처리 시, SplineComponent 및 PredictedEndPoint StaticMesh 또한 CreateDefaultSubobject로 생성해줄 것
	// TODO : Explosion Sphere (폭발 반경 Sphere) 는 만들어주어야 함
}

// Called when the game starts or when spawned
void AC_ThrowableWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	m_ProjectileMovement->Deactivate();
	
	m_MainCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics)
}

// Called every frame
void AC_ThrowableWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AC_ThrowableWeaponBase::AttachToHand(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false
	
	SetActorHiddenInGame(false);

	m_ProjectileMovement->Deactivate();
	// ClearSpline(); // PathSpline 예측 경로 기능이 있다 하면, Clear 한번 여기서 처리를 해주어야 함

	// Self init (이 변수들 처리 추후, 던지기 기다리기 처리 시 필요함)
	// bIsCharging       = false;
	// bIsOnThrowProcess = false;

	// Main HUD Throwable 종류로 초기화
	if (AC_UIManager* UIManager = Cast<AC_UIManager>(Player->GetController<APlayerController>()->GetHUD()))
		UIManager->GetMainHUDWidget()->ToggleAmmoInfoVisibility(true, EFireMode::Single, 1, 1);

	// 이 처리는 왜 해줬는지 잘 기억은 안남 (아마 Attach 하기전에 처리를 해주어야 똑바로 위치처리가 되어서 해주었던 것 같음)
	// TODO : 필요하다면 그때가서 풀기 (아마 딱히 필요 없어보임)
	// SetActorRelativeLocation(FVector::ZeroVector);
	
	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		m_HandSocketName
	);
	
	if (bIsAttached)
		Player->SetHandState(EHandState::WeaponThrowable);
	
	return bIsAttached;
}

bool AC_ThrowableWeaponBase::AttachToHolster(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	if (!Cast<AC_BasicPlayer>(_ParentMesh->GetOwner())) return false; // 무기집에 붙이려는 Actor가 Player형이 아닌 경우
	
	// 배그 모작에서,
	// 투척류를 핀까지만 뽑았고 쿠킹을 안했을 시 다시 집어넣음
	// 투척류를 안전손잡이까지 뽑았다면 현재 위치에 현재 투척류 그냥 바닥에 떨굼

	SetActorHiddenInGame(true);
	m_ProjectileMovement->Deactivate();

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
		s_HolsterSocketName
	);
	return bIsAttached;
}

bool AC_ThrowableWeaponBase::OnStartFire(class AC_BasicPlayer* _WeaponUser)
{
	return false;
}

bool AC_ThrowableWeaponBase::OnFireOnGoing(AC_BasicPlayer* _WeaponUser)
{
	return false;
}

bool AC_ThrowableWeaponBase::OnFireEnd(AC_BasicPlayer* _WeaponUser)
{
	return false;
}
