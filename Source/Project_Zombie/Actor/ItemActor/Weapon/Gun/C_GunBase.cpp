// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GunBase.h"
#include "Animation/AnimSequence.h"
#include "TimerManager.h"
#include "Engine/StaticMeshActor.h"
#include "DrawDebugHelpers.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "../WeaponComponent/GunComponent/C_GunDataTableComponent.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Actor/Character/NPC/Enemy/Zombie/Controller/C_ZombieController.h"
#include "Actor/Character/NPC/Enemy/Zombie/CopZombie/C_CopZombie.h"
#include "Actor/ItemActor/Weapon/WeaponComponent/GunComponent/C_AIGunUsageComponent.h"

#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

#include "GameModeAndManager/C_UIManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Utility/C_Util.h"

// 일단은 총기 오른손 부착 위치 Socket과 동일한 Socket으로 둠
const FName AC_GunBase::s_HandSocketName = TEXT("HandGrip_R");

AC_GunBase::AC_GunBase()
{
	PrimaryActorTick.bCanEverTick = true;

	m_Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = m_Collision;

	m_WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	m_WeaponMesh->SetupAttachment(RootComponent);

	m_WeaponMesh->SetLinearDamping(1.f);
	m_WeaponMesh->SetAngularDamping(1.f);
	
	m_DataCom = CreateDefaultSubobject<UC_GunDataTableComponent>(TEXT("DataComponent"));
	
	m_AIGunUsageComponent = CreateDefaultSubobject<UC_AIGunUsageComponent>(TEXT("AIGunUsageComponent"));
}

void AC_GunBase::BeginPlay()
{
	Super::BeginPlay();
	
	Gun_init();
	m_Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	m_WeaponMesh->SetMassOverrideInKg(NAME_None, 2000.f);
	m_WeaponMesh->SetLinearDamping(1.5f);
	m_WeaponMesh->SetAngularDamping(3.f);
}

void AC_GunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AC_GunBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	Gun_init();
}

void AC_GunBase::Gun_init()
{
	if (!m_DataCom) return;

	// 외형(Mesh) 로드
	if (USkeletalMesh* WeaponMeshAsset = Cast<USkeletalMesh>(m_DataCom->GetAssetData("WeaponSkeletalMesh").LoadSynchronous()))
	{
		if (m_WeaponMesh)
		{
			m_WeaponMesh->SetSkeletalMesh(WeaponMeshAsset);
		}
	}
	else
	{
		// 테이블에 에셋이 없을 때만 경고
		UE_LOG(LogTemp, Warning, TEXT("데이터 테이블에 WeaponMesh가 없음!"));
	}

	// 에디터 뷰포트에서 총기를 드래그해 움직일 때는 아래 '무거운 로직/수치 계산'을 패스
	// HasActorBegunPlay()는 실제 게임 플레이 버튼을 눌렀을 때만 true
	if (!HasActorBegunPlay())
	{
		return;
	}

	m_BaseDamage		= m_DataCom->GetData(TEXT("BaseDamage"));
	m_MaxAmmo			= m_DataCom->GetData(TEXT("MaxAmmo"));
	m_CurrentAmmo		= m_MaxAmmo;
	m_FireRate			= m_DataCom->GetData(TEXT("AttackRate"));
	m_ShellEjectImpulse = m_DataCom->GetData(TEXT("ShellEjectImpulse"));

	m_FireAnimation		= Cast<UAnimSequence>(m_DataCom->GetAssetData("FireAnimation").LoadSynchronous());
	m_ReloadAnimation	= Cast<UAnimSequence>(m_DataCom->GetAssetData("ReloadAnimation").LoadSynchronous());
	m_ShellMesh			= Cast<UStaticMesh>(m_DataCom->GetAssetData("ShellMesh").LoadSynchronous());

	if (!m_FireAnimation) { UE_LOG(LogTemp, Warning, TEXT("FireAnimation 로드 실패")); }
	if (!m_ReloadAnimation) { UE_LOG(LogTemp, Warning, TEXT("ReloadAnimation 로드 실패")); }
	if (!m_ShellMesh) { UE_LOG(LogTemp, Warning, TEXT("ShellMesh 로드 실패")); }

}

bool AC_GunBase::ConsumeAmmo()
{
	// 총알이 없다면 사격 중지
	if (m_CurrentAmmo <= 0)
	{
		if (AC_UIManager* UIManager = Cast<AC_UIManager>(GetWorld()->GetFirstPlayerController()->GetHUD()))
			UIManager->GetMainHUDWidget()->AddPlayerWarningLog("OUT OF AMMO");

		ReleaseTrigger();
		return false;
	}

	m_CurrentAmmo--;

	// 현재 남은 장탄수 UI 업데이트
	if (AC_UIManager* UIManager = Cast<AC_UIManager>(GetWorld()->GetFirstPlayerController()->GetHUD()))
		UIManager->GetMainHUDWidget()->UpdateMagazineAmmoCount(m_CurrentAmmo);

	return true;
}

void AC_GunBase::SpawnShellEject()
{
	if (m_ShellMesh && m_WeaponMesh && GetWorld())
	{
		FTransform EjectTransform = m_WeaponMesh->GetSocketTransform(TEXT("AmmoEject"), RTS_World);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* SpawnedShell = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), EjectTransform, SpawnParams);
		if (SpawnedShell)
		{
			UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(SpawnedShell, TEXT("ShellMeshComp"));
			if (MeshComp)
			{
				MeshComp->SetStaticMesh(m_ShellMesh);
				MeshComp->SetMobility(EComponentMobility::Movable);
				SpawnedShell->SetRootComponent(MeshComp);
				MeshComp->RegisterComponent();
				SpawnedShell->SetActorTransform(EjectTransform);

				MeshComp->SetSimulatePhysics(true);
				MeshComp->SetCollisionProfileName(TEXT("Custom"));
				MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
				MeshComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
				MeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
				MeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

				float RandomRightForce = FMath::FRandRange(130.0f, 220.0f);
				float RandomUpForce = FMath::FRandRange(60.0f, 130.0f);
				float RandomForwardForce = FMath::FRandRange(-40.0f, 40.0f);

				FVector EjectDirection = (EjectTransform.GetRotation().GetRightVector() * RandomRightForce)
					+ (EjectTransform.GetRotation().GetUpVector() * RandomUpForce)
					+ (EjectTransform.GetRotation().GetForwardVector() * RandomForwardForce);

				MeshComp->AddImpulse(EjectDirection, NAME_None, true);

				FVector RandomTorque = FVector(FMath::FRandRange(-50.0f, 50.0f), FMath::FRandRange(-50.0f, 50.0f), FMath::FRandRange(-50.0f, 50.0f));
				MeshComp->AddAngularImpulseInRadians(RandomTorque, NAME_None, true);

				SpawnedShell->SetLifeSpan(3.0f);
			}
		}
	}
}

void AC_GunBase::ProcessLineTraceDamage(float DamageVal)
{
	if (m_WeaponMesh && GetWorld())
	{
		FVector StartLocation = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
		FVector ForwardDirection = m_WeaponMesh->GetSocketRotation(TEXT("MuzzleFlash")).Vector();
		FVector MaxEndLocation = StartLocation + (ForwardDirection * 5000.0f); // TODO : 현재 사거리 50m로 고정되어 있음

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(m_WeaponPlayerUser);

		bool bHasHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, MaxEndLocation, ECC_Visibility, QueryParams);
		FVector ActualEndLocation = bHasHit ? HitResult.ImpactPoint : MaxEndLocation;

		DrawDebugLine(GetWorld(), StartLocation, ActualEndLocation, FColor::Green, false, 0.5f, 0, 1.5f);

		if (bHasHit)
		{
			DrawDebugSphere(GetWorld(), ActualEndLocation, 7.0f, 12, FColor::Red, false, 0.5f, 0, 1.5f);

			if (AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(HitResult.GetActor()))
			{
				UGameplayStatics::ApplyDamage(Enemy, DamageVal, m_WeaponPlayerUser->GetController(), this, nullptr);
			}
		}
	}
}

bool AC_GunBase::AttachToHand(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false

	m_WeaponPlayerUser = Player;

	// Main HUD MeleeWeapon 종류로 초기화
	if (APlayerController* PC = Player->GetController<APlayerController>())
	{
		// TODO : 각 MeleeWeapon에 맞는 이미지 아이콘(?) 표시해주면 좋을 듯 (일단은 AmmoInfo쪽 정보 감추는 처리로 함)
		if (AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD()))
			UIManager->GetMainHUDWidget()->ToggleAmmoInfoVisibility(true, EFireMode::FullAuto, m_CurrentAmmo, m_MaxAmmo); // TODO : FireMode 현재 FireMode로 넣어줄 것
	}

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		s_HandSocketName
	);
	
	if (bIsAttached)
		Player->SetHandState(EHandState::WeaponGun);
	
	return bIsAttached;
}

bool AC_GunBase::AttachToHolster(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		m_HolsterSocketName
	);
	
	return bIsAttached;
}

bool AC_GunBase::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	return false;
}

bool AC_GunBase::OnFireOnGoing(AC_BasicPlayer* _WeaponUser)
{
	return false;
}

bool AC_GunBase::OnFireEnd(AC_BasicPlayer* _WeaponUser)
{
	return false;
}

bool AC_GunBase::Reload(AC_BasicPlayer* _WeaponUser)
{
	return false;
}
