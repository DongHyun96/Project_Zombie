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
#include "Actor/Components/ItemLinkComponent/C_ItemLinkComponent.h"
#include "Actor/ItemActor/Weapon/WeaponComponent/GunComponent/AIGunUsageComponent/C_AIGunUsageComponent.h"

#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameModeAndManager/C_ItemManager.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Actor/Components/C_BasicPlayerAimComponent.h"
#include "Engine/AssetManager.h"

#include "GameModeAndManager/C_UIManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Utility/C_Util.h"

// 일단은 총기 오른손 부착 위치 Socket과 동일한 Socket으로 둠
const FName AC_GunBase::s_HandSocketName = TEXT("HandGrip_R");

// 모두 RifleHolster를 사용할 예정
const FName AC_GunBase::s_HolsterSocketName = TEXT("RifleHolster");

AC_GunBase::AC_GunBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SetReplicates(true);
	
	m_Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = m_Collision;

	m_WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	m_WeaponMesh->SetupAttachment(RootComponent);

	//m_WeaponMesh->replicate

	m_WeaponMesh->SetLinearDamping(1.f);
	m_WeaponMesh->SetAngularDamping(1.f);
	
	//m_DataCom = CreateDefaultSubobject<UC_GunDataTableComponent>(TEXT("DataComponent"));
	
	m_AIGunUsageComponent = CreateDefaultSubobject<UC_AIGunUsageComponent>(TEXT("AIGunUsageComponent"));
	

}

void AC_GunBase::BeginPlay()
{
	Super::BeginPlay();
	
	//Gun_init();
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

	//Gun_init();
}

bool AC_GunBase::InitializeItemActor(const FWeaponData* InRawData)
{
	//return Super::InitializeItemActor(InRawData);
	
	PRINT_LOCAL(GetWorld(), "InitializeItemActor", FColor::Red, 5.0f);
	
	const FGunData* GunData = static_cast<const FGunData*>(InRawData);
	
	if (!GunData)
	{
		UC_Util::Print("Failed Cast to const FGunData*", FColor::Red, 10.f);
		return false;
	}
	
	InitializeItemData(InRawData);
	
	LoadAsyncAssets(GunData);
	
	return true;
}

void AC_GunBase::InitializeItemData(const FWeaponData* InRawData)
{
	const FGunData* GunData = static_cast<const FGunData*>(InRawData);

	if (!GunData)
	{
		UC_Util::Print("Failed Cast to const FGunData*", FColor::Red, 10.f);
		return;
	}

	// Base Stats 적용
	float BaseDamage = GunData->BaseDamage;
	int32 BaseMaxAmmo = GunData->MaxAmmo;
	m_FireRate = GunData->AttackRate;
	m_ShellEjectImpulse = GunData->ShellEjectImpulse;

	if (!ItemLinkComp)
	{
		UC_Util::Print("GunBase : Item Link Component Is Nullptr!", FColor::Red, 10.f);
	}

	// 동적 데이터(CustomData) 처리
	if (FInventoryEntry* EntryPtr = ItemLinkComp ? ItemLinkComp->GetItemEntryPtr() : nullptr)
	{
		// 1. 없으면 데이터 안전하게 생성
		FEquipmentCustomData* CustomData = EntryPtr->GetOrCreateEquipmentData();

		// 2. Grade(단계) 가져오기
		int32 DamageGrade = CustomData->GetStatGrade(EUpgradableStats::AttackPower);
		int32 AmmoGrade = CustomData->GetStatGrade(EUpgradableStats::MaxAmmo);

		// 3. 최종 스탯 계산: BaseStat + (Grade * DataAsset의 레벨당 증가량)
		m_Damage = GunData->BaseDamage + (DamageGrade * GunData->DamagePerUpgradeLevel);
		m_MaxAmmo = GunData->MaxAmmo + (AmmoGrade * GunData->MaxAmmoPerUpgradeLevel);
		m_FireRate = GunData->AttackRate - (DamageGrade * GunData->AttackRatePerUpgradeLevel);
		// 4. 현재 탄약 정보 적용 (초기 세팅 상태라면 MaxAmmo로 설정)
		//m_CurrentAmmo = CustomData->CurAmmo > 0 ? CustomData->CurAmmo : m_MaxAmmo;
	}
	else
	{
		// Link가 무효한 상태일 때 Base 수치 적용
		m_Damage = GunData->BaseDamage;
		m_MaxAmmo = GunData->MaxAmmo;
		//m_CurrentAmmo = m_MaxAmmo;
	}
	//CurAmmo는 무조건 0으로 초기화 하기.
	m_CurrentAmmo = 0;
}

void AC_GunBase::SwitchFireMode()
{
	if (m_OwnerPlayer && m_OwnerPlayer->IsLocallyControlled())
		UI_MANAGER(GetWorld())->GetMainHUDWidget()->UpdateFireMode(m_FireMode);
}

void AC_GunBase::LoadAsyncAssets(const FWeaponData* InRawData)
{
	//Super::LoadAsyncAssets(InRawData);
	
	PRINT_LOCAL(GetWorld(), "Start LoadAsyncAssets", FColor::Red, 10.f);
	
	const FGunData* GunData = static_cast<const FGunData*>(InRawData);
	
	if (!GunData)
	{
		UC_Util::Print("Failed Cast to const FGunData*", FColor::Red, 10.f);
		return;
	}
	
	PRINT_LOCAL(GetWorld(), "Playing LoadAsyncAssets", FColor::Red, 10.f);
	// 기존에 요청 중이던 이 객체의 비동기 로드 취소.
	CancelAsyncLoad();
	
	// TODO : 비동기 로드의 특성상 무기를 들고 있는 상태에서 아이템이 바뀌게 된다면 IDLE 상태로 전환하고 총을 다시 꺼내는 방식으로 가는게 안전해 보임.
	// TODO : 만약 이것도 상황이 여의치 않다면 그냥 장비 슬롯이 아니라 인벤에 들어 올 때 해당 장비의 에셋들을 비동기로드를 미리 처리하는 방법도 있음.
	// 비동기로 로드할 SoftObjectPath 목록 수집
	TArray<FSoftObjectPath> AssetsToLoad;

	if (!GunData->WeaponSkeletalMesh.IsNull()) AssetsToLoad.Add(GunData->WeaponSkeletalMesh.ToSoftObjectPath());
	if (!GunData->FireAnimation.IsNull()) AssetsToLoad.Add(GunData->FireAnimation.ToSoftObjectPath());
	if (!GunData->ReloadAnimation.IsNull()) AssetsToLoad.Add(GunData->ReloadAnimation.ToSoftObjectPath());
	if (!GunData->ShellMesh.IsNull()) AssetsToLoad.Add(GunData->ShellMesh.ToSoftObjectPath());
	if (!GunData->PlayerReloadAnimation.IsNull()) AssetsToLoad.Add(GunData->PlayerReloadAnimation.ToSoftObjectPath());
	if (!GunData->PlayerFireAnimation.IsNull()) AssetsToLoad.Add(GunData->PlayerFireAnimation.ToSoftObjectPath());
	PRINT_LOCAL(GetWorld(), "Success LoadAsyncAssets", FColor::Red, 10.f);
	// AssetManager를 통한 비동기 로딩 요청
	if (AssetsToLoad.Num() > 0)
	{
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		
		// 캡처용 변수 복사
		TSoftObjectPtr<USkeletalMesh> SoftMesh = GunData->WeaponSkeletalMesh;
		TSoftObjectPtr<UAnimSequence> SoftFireAnim = GunData->FireAnimation;
		TSoftObjectPtr<UAnimSequence> SoftReloadAnim = GunData->ReloadAnimation;
		TSoftObjectPtr<UStaticMesh> SoftShellMesh = GunData->ShellMesh;
		TSoftObjectPtr<UAnimMontage> SoftPlayerReloadAnim = GunData->PlayerReloadAnimation;
		TSoftObjectPtr<UAnimMontage> SoftPlayerFireAnim = GunData->PlayerFireAnimation;

		// 람다(Lambda)를 이용해 로딩이 완료된 시점에 포인터 캐싱 및 메시 적용
		Streamable.RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateLambda([this, SoftMesh, SoftFireAnim, SoftReloadAnim, SoftShellMesh, SoftPlayerReloadAnim, SoftPlayerFireAnim]()
		{
			if (!IsValid(this)) return;

			if (SoftMesh.IsValid() && m_WeaponMesh)
			{
				m_WeaponMesh->SetSkeletalMesh(SoftMesh.Get());
			}

			m_FireAnimation = SoftFireAnim.Get();
			m_ReloadAnimation = SoftReloadAnim.Get();
			m_ShellMesh = SoftShellMesh.Get();
			m_PlayerReloadAnimation = SoftPlayerReloadAnim.Get();
			m_PlayerFireAnimation = SoftPlayerFireAnim.Get();

			UC_Util::Print("Weapon Assets Async Loaded Successfully!", FColor::Green, 5.f);
		}));
	}
	
	/*// 1. OwnerPlayer 유효성 검사
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled())
	{
		return;
	}

	// 2. PlayerController 가져오기 및 검사
	APlayerController* PC = Cast<APlayerController>(m_OwnerPlayer->GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] PlayerController가 유효하지 않습니다."), *GetName());
		return;
	}

	// 3. HUD(AC_UIManager) 가져오기 및 검사
	AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD());
	if (!UIManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] UIManager(HUD)를 찾을 수 없거나 형변환에 실패했습니다."), *GetName());
		return;
	}

	// 4. MainHUDWidget 가져오기 및 검사
	UC_GameMainHUD* MainWidget = UIManager->GetMainHUDWidget(); // 실제 반환 타입 클래스로 맞춰주세요
	if (!MainWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] MainHUDWidget이 아직 생성되지 않았습니다."), *GetName());
		return;
	}

	// 5. 모든 포인터가 유효할 때만 안전하게 실행
	MainWidget->ToggleAmmoInfoVisibility(true, EFireMode::FullAuto, m_CurrentAmmo, m_MaxAmmo);*/
}

void AC_GunBase::SetAmmoUIInfo(FAmmoUIInfo& _AmmoUIInfo)
{
	_AmmoUIInfo.Visible            = true;
	_AmmoUIInfo.FireMode           = m_FireMode;
	_AmmoUIInfo.MagazineAmmo       = m_CurrentAmmo;
	_AmmoUIInfo.LeftAmmoTotalCount = m_MaxAmmo;
}

/*void AC_GunBase::Gun_init()
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
}*/

bool AC_GunBase::ConsumeAmmo()
{
	if (m_CurrentAmmo > 0)
	{
		m_CurrentAmmo--;

		if (HasAuthority() && m_OwnerPlayer && m_OwnerPlayer->IsLocallyControlled())
		{
			OnRep_CurrentAmmo();
		}

		return true;
	}
	return false;
}

void AC_GunBase::PlayFireEffects_Local()
{
	if (m_OwnerPlayer && m_PlayerFireAnimation)
	{
		m_OwnerPlayer->PlayAnimMontage(m_PlayerFireAnimation);
	}

	if (m_WeaponMesh && m_FireAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_FireAnimation, false);
	}
}

void AC_GunBase::Client_PlayFireEffects_Implementation()
{
	if (m_OwnerPlayer && m_OwnerPlayer->IsLocallyControlled()) return;
	PlayFireEffects_Local();
}

void AC_GunBase::Multicast_PlayReloadEffects_Implementation()
{
	if (m_WeaponMesh && m_ReloadAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_ReloadAnimation, false);
	}

	if (m_OwnerPlayer && m_PlayerReloadAnimation)
	{
		m_OwnerPlayer->PlayAnimMontage(m_PlayerReloadAnimation);
	}
}

void AC_GunBase::OnRep_CurrentAmmo()
{
	if (m_OwnerPlayer && m_OwnerPlayer->IsLocallyControlled())
	{
		UI_MANAGER(GetWorld())->GetMainHUDWidget()->UpdateMagazineAmmoCount(m_CurrentAmmo);
	}
}

void AC_GunBase::SpawnShellEject()
{
	if (!m_ShellEjectNiagaraSystem || !m_ShellMesh || !m_WeaponMesh || !GetWorld()) return;

	FTransform EjectTransform = m_WeaponMesh->GetSocketTransform(TEXT("AmmoEject"), RTS_World);

	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		m_ShellEjectNiagaraSystem,
		EjectTransform.GetLocation(),
		EjectTransform.GetRotation().Rotator()
	);

	if (NiagaraComp)
	{
		NiagaraComp->SetVariableStaticMesh(FName("ShellMesh"), m_ShellMesh);
		NiagaraComp->SetIntParameter(FName("ShellCount"), 1);
	}
}

void AC_GunBase::Multicast_PlayFireEffects_Implementation(const FVector_NetQuantize& ImpactPoint)
{
	if (m_OwnerPlayer && m_PlayerFireAnimation)
	{
		m_OwnerPlayer->PlayAnimMontage(m_PlayerFireAnimation);
	}

	if (m_WeaponMesh && m_FireAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_FireAnimation, false);
	}

	if (m_OwnerPlayer && !m_OwnerPlayer->IsLocallyControlled())
	{
		PlayFireEffects_Local();
	}

	SpawnShellEject();

	if (m_WeaponMesh && GetWorld())
	{
		FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
		FVector ExplicitImpactPoint = FVector(ImpactPoint);
		FVector ShootDir = (ExplicitImpactPoint - MuzzleStart).GetSafeNormal();
		FRotator MuzzleRotation = ShootDir.Rotation();

		if (m_TracerFX)
		{
			UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				m_TracerFX,
				MuzzleStart,
				MuzzleRotation,
				FVector(1.0f),
				false
			);

			if (TracerComp)
			{
				float Distance = FVector::Distance(MuzzleStart, ExplicitImpactPoint);
				float Speed = 20000.0f;
				float FlyTime = Distance / Speed;

				TSharedPtr<float> ElapsedTime = MakeShared<float>(0.0f);
				TSharedPtr<FTimerHandle> TracerTimerHandle = MakeShared<FTimerHandle>();

				GetWorld()->GetTimerManager().SetTimer(
					*TracerTimerHandle,
					[TracerComp, MuzzleStart, ExplicitImpactPoint, ShootDir, FlyTime, ElapsedTime, TracerTimerHandle, this]() mutable
					{
						if (!TracerComp || !TracerComp->IsValidLowLevel()) return;

						*ElapsedTime += 0.01f;
						float Alpha = FMath::Clamp(*ElapsedTime / FlyTime, 0.0f, 1.0f);

						FVector CurrentLoc = FMath::Lerp(MuzzleStart, ExplicitImpactPoint, Alpha);
						TracerComp->SetWorldLocation(CurrentLoc);

						if (Alpha >= 1.0f)
						{
							TracerComp->DeactivateSystem();
							TracerComp->DestroyComponent();

							if (m_ImpactFX && GetWorld())
							{
								FRotator ImpactRotation = (-ShootDir).Rotation();
								UGameplayStatics::SpawnEmitterAtLocation(
									GetWorld(),
									m_ImpactFX,
									ExplicitImpactPoint,
									ImpactRotation,
									FVector(1.0f),
									true
								);
							}

							if (GetWorld() && TracerTimerHandle.IsValid())
							{
								GetWorld()->GetTimerManager().ClearTimer(*TracerTimerHandle);
							}
						}
					},
					0.01f,
					true
				);
			}
		}
	}
}
/*
void AC_GunBase::InitFromInventoryEntry(const FInventoryEntry& InEntry)
{
	ItemEntry = InEntry;
	//Gun_init();
}
*/

/*FInventoryEntry AC_GunBase::GetUpdatedInventoryEntry()
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
}*/

void AC_GunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AC_GunBase, m_Damage);
	DOREPLIFETIME(AC_GunBase, m_CurrentAmmo);
	DOREPLIFETIME(AC_GunBase, m_MaxAmmo);
	DOREPLIFETIME(AC_GunBase, m_FireRate);
	DOREPLIFETIME(AC_GunBase, m_bIsReloading);
}

bool AC_GunBase::AttachToHand(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 손에 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false
	if (Player != m_OwnerPlayer) return false; // 손에 장착 시도하는 Player가 무기주인인 경우가 아닌 경우

	SetOwner(Player);

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
		s_HandSocketName
	);
	
	if (bIsAttached)
	{
		Player->SetHandState(EHandState::WeaponGun);
		UpdateAmmoInfoHUDForDrawEnd();
	}
	else PRINT_LOCAL(GetWorld(), "AttachToHand Failed", FColor::Red, 10.f);
	
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
		s_HolsterSocketName
	);
	
	if (bIsAttached)
	{
		m_OwnerPlayer = Player;
		
		// Reload 과정 초기화 -> Reload 완수 이전이라면, 완수할 수 없게끔
		// 카메라 원위치 (견착조준 상태였다면)
		// FireWeapon -> 사격 도중 끊김 : 연발 사격 시 계속해서 Timer 등록처리됨 -> 이거는 근데 AttachToHolster 시점이 아닌, SheathWeapon 처리 시 바로 들어가줘야 할듯
	}
	
	return bIsAttached;
}

bool AC_GunBase::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser) return false;
	m_OwnerPlayer = _WeaponUser;

	if (m_bIsReloading) return false;

	PullTrigger();
	return true;
}

bool AC_GunBase::OnFireOnGoing(AC_BasicPlayer* _WeaponUser)
{
	return false;
}

bool AC_GunBase::OnFireEnd(AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser) return false;
	ReleaseTrigger();
	return true;
}

bool AC_GunBase::Reload(AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser) return false;
	m_OwnerPlayer = _WeaponUser;

	if (m_CurrentAmmo >= m_MaxAmmo || m_bIsReloading) return false;

	Server_StartReload();
	return true;
}

void AC_GunBase::UpdateAmmoInfoHUDForDrawEnd()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled()) return;

	UI_MANAGER(GetWorld())->GetMainHUDWidget()->ToggleAmmoInfoVisibility(true, m_FireMode, m_CurrentAmmo, m_MaxAmmo);
}

void AC_GunBase::PullTrigger()
{
	// 재장전 중(m_bIsReloading)이거나 탄약이 없으면 발사 불가
	if (m_bIsFiring || m_bIsReloading || m_CurrentAmmo <= 0) return;

	m_bIsFiring = true;
	PlayFireEffects_Local();
	Server_PullTrigger();
}

void AC_GunBase::ReleaseTrigger()
{
	m_bIsFiring = false;
	Server_ReleaseTrigger();
}

FVector AC_GunBase::LineTraceDamage(const FVector& CameraStart, const FRotator& CameraRot, float DamageVal, float SpreadAngleDegree)
{
	if (!m_WeaponMesh || !GetWorld() || !m_OwnerPlayer)
		return FVector::ZeroVector;

	FVector CameraForward = CameraRot.Vector();
	float TraceRange = 10000.0f;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(m_OwnerPlayer);

	FVector CameraEnd = CameraStart + (CameraForward * TraceRange);
	FHitResult CameraHitResult;

	bool bCameraHit = GetWorld()->LineTraceSingleByChannel(
		CameraHitResult,
		CameraStart,
		CameraEnd,
		ECC_Visibility,
		QueryParams
	);

	FVector TargetPoint = bCameraHit ? CameraHitResult.ImpactPoint : CameraEnd;
	FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector ShootDirection = (TargetPoint - MuzzleStart).GetSafeNormal();

	if (SpreadAngleDegree > 0.0f)
	{
		float ConeHalfAngleRad = FMath::DegreesToRadians(SpreadAngleDegree);
		ShootDirection = FMath::VRandCone(ShootDirection, ConeHalfAngleRad);
	}

	FVector FinalMuzzleEnd = MuzzleStart + (ShootDirection * TraceRange);
	FHitResult MuzzleHitResult;

	bool bMuzzleHit = GetWorld()->LineTraceSingleByChannel(
		MuzzleHitResult,
		MuzzleStart,
		FinalMuzzleEnd,
		ECC_Visibility,
		QueryParams
	);

	FVector ActualEndLocation = bMuzzleHit ? MuzzleHitResult.ImpactPoint : FinalMuzzleEnd;

	if (bMuzzleHit)
	{
		if (AC_BasicEnemy* Enemy = Cast<AC_BasicEnemy>(MuzzleHitResult.GetActor()))
		{
			AController* InstigatorController = m_OwnerPlayer->GetController();
			UGameplayStatics::ApplyDamage(Enemy, DamageVal, InstigatorController, this, nullptr);
		}
	}

	return ActualEndLocation;
}

void AC_GunBase::Server_PullTrigger_Implementation()
{
	if (m_bIsReloading || m_CurrentAmmo <= 0) return;

	if (ConsumeAmmo())
	{
		Server_ExecuteFire();
	}
}

void AC_GunBase::Server_ReleaseTrigger_Implementation()
{
	m_bIsFiring = false;
}

void AC_GunBase::Server_ExecuteFire()
{
	if (!m_OwnerPlayer) return;

	FVector CameraLoc;
	FRotator CameraRot;
	m_OwnerPlayer->GetActorEyesViewPoint(CameraLoc, CameraRot);

	FVector ImpactPoint = LineTraceDamage(CameraLoc, CameraRot, m_Damage, 0.0f);

	Multicast_PlayFireEffects(ImpactPoint);
}

void AC_GunBase::Server_StartReload_Implementation()
{
	if (m_CurrentAmmo >= m_MaxAmmo || m_bIsReloading) return;

	m_bIsReloading = true;
	ReleaseTrigger();

	Multicast_PlayReloadEffects();

	float ReloadDuration = 0.0f;
	if (m_PlayerReloadAnimation)
	{
		ReloadDuration = m_PlayerReloadAnimation->GetPlayLength();
	}

	GetWorld()->GetTimerManager().SetTimer(
		m_ReloadTimerHandle,
		this,
		&AC_GunBase::Server_ExecuteReload,
		ReloadDuration,
		false
	);
}

void AC_GunBase::Server_ExecuteReload()
{
	m_CurrentAmmo = m_MaxAmmo;
	m_bIsReloading = false;

	OnRep_CurrentAmmo();
}

void AC_GunBase::OnSheathStart()
{
	ReleaseTrigger(); // 사격 중이었다면, 사격 중지 (Timer 등록 해지 등)

	// Aim 카메라, UI 등 원위치
	if (m_OwnerPlayer) m_OwnerPlayer->GetAimComponent()->OnAimReleased();
	
	// TODO : 재장전 중이었던 경우, 필요한 원위치 또는 중단 처리 필요
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

	AC_BasicPlayer* OverlappedPlayer = Cast<AC_BasicPlayer>(_OtherActor);
	if (!OverlappedPlayer) return; // Player가 아닌 다른 물체와 Overlap
	
	// TODO : 리슨서버에서의 아이템 파밍 시퀀스를 따라서 처리를 할 것
	// 일단은 LocalPlayer인 경우에만, 충돌처리를 하는 것으로 체킹함
	if (!OverlappedPlayer->IsLocallyControlled()) return;
	
	// 해당 Player의 MainWeaponSlot에 이미 MainWeapon이 장착되어 있는 경우
	if (OverlappedPlayer->GetEquippedComponent()->GetSlotWeapon(EWeaponSlot::MainWeapon)) return;
	
	OverlappedPlayer->GetEquippedComponent()->Server_SetSlotWeapon(EWeaponSlot::MainWeapon, this);
	m_Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Outline 비활성화(활성화 되어있건 이미 비활성이건)
	m_WeaponMesh->SetCustomDepthStencilValue(0);

	if (OverlappedPlayer->GetPingSystemComponent()->GetLastInstigator() == this)
	{
		// 아직 마지막으로 핑을 스폰한 LastInstigator가 이 총기일 경우 -> 아직 해당 정보의 Ping이 나온 상태
		// Ping 정보를 가려준다
		OverlappedPlayer->GetPingSystemComponent()->HidePing();
	}
}
