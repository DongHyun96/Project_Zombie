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
#include "Actor/Components/C_EquippedComponent.h"
#include "Actor/Components/C_PingSystemComponent.h"
#include "Actor/ItemActor/Weapon/WeaponComponent/GunComponent/C_AIGunUsageComponent.h"

#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameModeAndManager/C_ItemManager.h"

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
	m_Collision->OnComponentBeginOverlap.AddDynamic(this, &AC_GunBase::OnMainColliderBeginOverlap);
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
	if (ItemEntry.ItemRowName.IsNone()) return;

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UC_ItemManager* ItemManager = GI->GetSubsystem<UC_ItemManager>();
	if (!ItemManager) return;

	// 1. UC_ItemManager를 통해 데이터 테이블 항목 가져오기
	const FGunData* GunData = ItemManager->GetItemData<FGunData>(EItemTableType::Gun, ItemEntry.ItemRowName);
	if (!GunData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AC_GunBase] GunData를 찾을 수 없음: %s"), *ItemEntry.ItemRowName.ToString());
		return;
	}

	// 2. Mesh 및 애니메이션 리소스 로드/설정
	if (GunData->WeaponSkeletalMesh.IsValid() || !GunData->WeaponSkeletalMesh.IsNull())
	{
		USkeletalMesh* MeshAsset = GunData->WeaponSkeletalMesh.LoadSynchronous();
		if (m_WeaponMesh && MeshAsset)
		{
			m_WeaponMesh->SetSkeletalMesh(MeshAsset);
		}
	}

	// 게임 플레이 중이 아닐 때는 에셋 메쉬만 맞추고 리턴
	if (!HasActorBegunPlay()) return;

	m_PlayerReloadAnimation = Cast<UAnimMontage>(m_DataCom->GetAssetData("PlayerReloadAnimation").LoadSynchronous());
	m_PlayerFireAnimation = Cast<UAnimMontage>(m_DataCom->GetAssetData("PlayerFireAnimation").LoadSynchronous());
	// 3. 에셋 캐싱
	m_FireAnimation = GunData->FireAnimation.LoadSynchronous();
	m_ReloadAnimation = GunData->ReloadAnimation.LoadSynchronous();
	m_ShellMesh = GunData->ShellMesh.LoadSynchronous();

	// 4. Base Stats 적용
	float BaseDamage = GunData->BaseDamage;
	int32 BaseMaxAmmo = GunData->MaxAmmo;
	m_FireRate = GunData->AttackRate;
	m_ShellEjectImpulse = GunData->ShellEjectImpulse;

	// 5. CustomData(강화, 잔탄량) 처리
	if (const FGunCustomData* GunCustomData = ItemEntry.CustomData.GetPtr<FGunCustomData>())
	{
		m_Damage = BaseDamage + (GunCustomData->Upgrade_Damage * 5.0f);
		m_MaxAmmo = BaseMaxAmmo + (GunCustomData->Upgrade_MaxAmmo * 5);
		m_CurrentAmmo = GunCustomData->CurAmmo;
	}
	else
	{
		// CustomData가 없는 초기 아이템 상태
		m_Damage = BaseDamage;
		m_MaxAmmo = BaseMaxAmmo;
		m_CurrentAmmo = m_MaxAmmo;
	}
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
	if (!m_ShellMesh || !m_WeaponMesh || !GetWorld()) return;

    FTransform EjectTransform = m_WeaponMesh->GetSocketTransform(TEXT("AmmoEject"), RTS_World);
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // AStaticMeshActor 사용으로 코드 간소화 및 스폰 안정성 향상
    AStaticMeshActor* ShellActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), EjectTransform, SpawnParams);
    if (ShellActor)
    {
        UStaticMeshComponent* MeshComp = ShellActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            MeshComp->SetStaticMesh(m_ShellMesh);
            MeshComp->SetMobility(EComponentMobility::Movable);
            MeshComp->SetSimulatePhysics(true);
        	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        	
            MeshComp->SetCollisionProfileName(TEXT("Custom"));
            MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
            MeshComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
            MeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            MeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

            float RandomRight = FMath::FRandRange(130.0f, 220.0f);
            float RandomUp = FMath::FRandRange(60.0f, 130.0f);
            float RandomForward = FMath::FRandRange(-40.0f, 40.0f);

            FVector EjectDir = (EjectTransform.GetRotation().GetRightVector() * RandomRight)
                            + (EjectTransform.GetRotation().GetUpVector() * RandomUp)
                            + (EjectTransform.GetRotation().GetForwardVector() * RandomForward);

            MeshComp->AddImpulse(EjectDir, NAME_None, true);

            FVector RandomTorque = FVector(
                FMath::FRandRange(-50.0f, 50.0f),
                FMath::FRandRange(-50.0f, 50.0f),
                FMath::FRandRange(-50.0f, 50.0f)
            );
            MeshComp->AddAngularImpulseInRadians(RandomTorque, NAME_None, true);

            ShellActor->SetLifeSpan(3.0f);
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
		QueryParams.AddIgnoredActor(m_OwnerPlayer);

		bool bHasHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, MaxEndLocation, ECC_Visibility, QueryParams);
		FVector ActualEndLocation = bHasHit ? HitResult.ImpactPoint : MaxEndLocation;

		DrawDebugLine(GetWorld(), StartLocation, ActualEndLocation, FColor::Green, false, 0.5f, 0, 1.5f);

		if (bHasHit)
		{
			DrawDebugSphere(GetWorld(), ActualEndLocation, 7.0f, 12, FColor::Red, false, 0.5f, 0, 1.5f);

			if (AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(HitResult.GetActor()))
			{
				UGameplayStatics::ApplyDamage(Enemy, DamageVal, m_OwnerPlayer->GetController(), this, nullptr);
			}
		}
	}
}

void AC_GunBase::InitFromInventoryEntry(const FInventoryEntry& InEntry)
{
	ItemEntry = InEntry;
	Gun_init();
}

FInventoryEntry AC_GunBase::GetUpdatedInventoryEntry()
{
	// 실시간으로 변한 수치(예: CurAmmo)를 CustomData에 다시 구겨넣어서 최신화
	FGunCustomData* GunCustomData = ItemEntry.CustomData.GetMutablePtr<FGunCustomData>();
	if (!GunCustomData)
	{
		// 없으면 새로 생성 후 대입
		FGunCustomData NewData;
		ItemEntry.CustomData = FInstancedStruct::Make(NewData);
		GunCustomData = ItemEntry.CustomData.GetMutablePtr<FGunCustomData>();
	}

	if (GunCustomData)
	{
		GunCustomData->CurAmmo = m_CurrentAmmo;
		// 필요 시 실시간 변화하는 추가 동적 수치들 업링크
	}

	return ItemEntry;
}

bool AC_GunBase::AttachToHand(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 손에 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false
	if (Player != m_OwnerPlayer) return false; // 손에 장착 시도하는 Player가 무기주인인 경우가 아닌 경우 

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
		FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
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
		FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
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

void AC_GunBase::OnMainColliderBeginOverlap
(
	UPrimitiveComponent* _OverlapComponent,
	AActor*				 _OtherActor,
	UPrimitiveComponent* _OtherComp,
	int32				 _OtherBodyIndex,
	bool				 _bFromSweep,
	const FHitResult&	 _SweepResult
)
{
	/* 무기를 줍는 처리 */

	// 이미 이 무기의 주인이 존재
	if (m_OwnerPlayer) return;

	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_OtherActor);
	if (!Player) return; // Player가 아닌 다른 물체와 Overlap
	
	// 해당 Player의 MainWeaponSlot에 이미 MainWeapon이 장착되어 있는 경우
	if (Player->GetEquippedComponent()->GetSlotWeapon(EWeaponSlot::MainWeapon)) return;
	
	Player->GetEquippedComponent()->SetSlotWeapon(EWeaponSlot::MainWeapon, this);
	m_Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Outline 비활성화(활성화 되어있건 이미 비활성이건)
	m_WeaponMesh->SetCustomDepthStencilValue(0);

	if (Player->GetPingSystemComponent()->GetLastInstigator() == this)
	{
		// 아직 마지막으로 핑을 스폰한 LastInstigator가 이 총기일 경우 -> 아직 해당 정보의 Ping이 나온 상태
		// Ping 정보를 가려준다
		Player->GetPingSystemComponent()->HidePing();
	}
}