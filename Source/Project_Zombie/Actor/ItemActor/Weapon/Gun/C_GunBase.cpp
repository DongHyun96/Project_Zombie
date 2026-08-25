// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GunBase.h"
#include "Animation/AnimSequence.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Actor/Components/C_EquippedComponent.h"
#include "Actor/Components/ItemLinkComponent/C_ItemLinkComponent.h"
#include "Actor/ItemActor/Weapon/WeaponComponent/GunComponent/AIGunUsageComponent/C_AIGunUsageComponent.h"

#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

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
}

void AC_GunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
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
	if (!ItemLinkComp)
	{
		UC_Util::Print("GunBase : Item Link Component Is Nullptr!", FColor::Red, 10.f);
	}

	// 동적 데이터(CustomData) 처리
	if (FInventoryEntry* EntryPtr = ItemLinkComp ? ItemLinkComp->GetItemEntryPtr() : nullptr)
	{
		// 1. 없으면 데이터 안전하게 생성
		FUpgradableData* CustomData = EntryPtr->GetOrCreateEquipmentData();

		// 2. Grade(단계) 가져오기
		int32 DamageGrade = CustomData->GetStatGrade(EUpgradableStats::AttackPower);
		int32 AmmoGrade = CustomData->GetStatGrade(EUpgradableStats::MaxAmmo);

		// 3. 최종 스탯 계산: BaseStat + (Grade * DataAsset의 레벨당 증가량) // TODO : DT_WEaponUpgradeData를 사용하게 바꿔야 함. FWeaponUpgradeData 자체를 바꿔야 할 수 도 있음.
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
	
	FString msg = FString::SanitizeFloat(m_Damage);
	
	PRINT_LOCAL(GetWorld(), msg, FColor::Black, 10.f);
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

		// 비동기 요청
		m_AsyncLoadHandle = Streamable.RequestAsyncLoad(
		   AssetsToLoad,
		   FStreamableDelegate::CreateWeakLambda(this, [this, SoftMesh, SoftFireAnim, SoftReloadAnim, SoftShellMesh, SoftPlayerReloadAnim, SoftPlayerFireAnim]()
		   {
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
		   })
		);
	}
}

void AC_GunBase::SetAmmoUIInfo(FAmmoUIInfo& _AmmoUIInfo)
{
	_AmmoUIInfo.Visible            = true;
	_AmmoUIInfo.FireMode           = m_FireMode;
	_AmmoUIInfo.MagazineAmmo       = m_CurrentAmmo;
	_AmmoUIInfo.LeftAmmoTotalCount = m_MaxAmmo;
}

void AC_GunBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
		GetWorldTimerManager().ClearTimer(m_FireTimerHandle);
	
	Super::EndPlay(EndPlayReason);
}

bool AC_GunBase::PlayFireEffects()
{
	if (!m_OwnerPlayer)
	{
		UC_Util::Print("[AC_GunBase::PlayFireEffects] : OwnerPlayer nullptr]", FColor::Red, 10.f);
		return false;
	}
	
	if (m_PlayerFireAnimation)
	{
		const float Duration = m_OwnerPlayer->PlayAnimMontage(m_PlayerFireAnimation);
		if (Duration <= 0.f) return false;
	}
	else UC_Util::Print("[AC_GunBase::PlayFireEffects] (" + GetName() + ") : FireAnimation nullptr", FColor::Red, 10.f);

	if (m_WeaponMesh && m_FireAnimation)
		m_WeaponMesh->PlayAnimation(m_FireAnimation, false);
	
	return true;
}

void AC_GunBase::Multicast_PlayReloadEffects_Implementation()
{
	if (!m_OwnerPlayer)
	{
		UC_Util::Print("[AC_GunBase::Multicast_PlayReloadEffects] : OwnerPlayer nullptr", FColor::Red, 10.f);
		return;
	}
	
	if (m_OwnerPlayer->IsLocallyControlled()) return;
	
	if (m_WeaponMesh && m_ReloadAnimation)
		m_WeaponMesh->PlayAnimation(m_ReloadAnimation, false);

	if (m_PlayerReloadAnimation)
		m_OwnerPlayer->PlayAnimMontage(m_PlayerReloadAnimation);
}

void AC_GunBase::UpdateAmmoUI()
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

void AC_GunBase::Multicast_PlayFireEffects_Implementation(FVector_NetQuantize ImpactPoint)
{
	 if (!m_OwnerPlayer) return;

	// 이미 발사 당사자는 PlayFireEffects 관련 처리를 진행한 상황(중복 방지)
	if (!m_OwnerPlayer->IsLocallyControlled())
	{
		PlayFireEffects();
		SpawnShellEject();  
	}

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

void AC_GunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AC_GunBase, m_Damage);
	DOREPLIFETIME(AC_GunBase, m_MaxAmmo);
}

// 타이머 중단 및 총기 상태값 RPC
void AC_GunBase::Server_CancelReload_Implementation()
{
	Multicast_CancelReload();
}

void AC_GunBase::Multicast_CancelReload_Implementation()
{
	// 모든 클라이언트들의 재장전 애니메이션 정지를 위한 멀티캐스트
	// 자기자신은 이미 해당 행위 Interrupt 및 정지 처리가 이루어짐
	if (!m_OwnerPlayer)
	{
		UC_Util::Print("[AC_GunBase::Multicast_StopReloadEffects] : OwnerPlayer nullptr", FColor::Red, 10.f);
		return;
	}

	// 자기자신의 Animation은 이미 끊어버린 상황
	if (m_OwnerPlayer->IsLocallyControlled()) return;
	
	if (m_PlayerReloadAnimation)
		m_OwnerPlayer->StopAnimMontage(m_PlayerReloadAnimation);

	if (m_WeaponMesh && m_ReloadAnimation)
		m_WeaponMesh->Stop();
}

void AC_GunBase::AN_OnGunReloadEnd()
{
	if (!m_OwnerPlayer) return;
	if (!m_OwnerPlayer->IsLocallyControlled()) return; // 로컬 플레이어 자기자신의 장탄수만 업데이트 처리하면 됨
	
	m_CurrentAmmo  = m_MaxAmmo;
	m_bIsReloading = false;
	m_bIsFiring    = false;

	UpdateAmmoUI(); // locallyController player 자기자신의 UI 갱신
}

bool AC_GunBase::AttachToHand(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh)
	{
		PRINT_LOCAL(GetWorld(), "AttachToHand ParentMesh Nullptr", FColor::Cyan, 10.f);
		return false;
	}
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player)
	{
		PRINT_LOCAL(GetWorld(), "AttachToHand ParentMesh->GetOwner() Nullptr", FColor::Cyan, 10.f);
		return false; // 손에 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false
	}
	if (Player != m_OwnerPlayer)
	{
		PRINT_LOCAL(GetWorld(), "AttachToHand Player != m_OwnerPlayer", FColor::Cyan, 10.f);
		return false; // 손에 장착 시도하는 Player가 무기주인인 경우가 아닌 경우
	}

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
		s_HandSocketName
	);
	
	if (bIsAttached)
	{
		SetOwner(Player);
		m_OwnerPlayer = Player;
		
		Player->SetHandState(EHandState::WeaponGun);
		UpdateAmmoInfoHUDForDrawEnd();
	}
	else PRINT_LOCAL(GetWorld(), "AttachToComponent(hand) Failed", FColor::Red, 10.f);
	
	return bIsAttached;
}

bool AC_GunBase::AttachToHolster(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh)
	{
		PRINT_LOCAL(GetWorld(), "AttachToHolster ParentMesh Nullptr", FColor::Cyan, 10.f);
		return false;
	}
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player)
	{
		PRINT_LOCAL(GetWorld(), "ParentMeshes GetOwner() nullptr", FColor::Cyan, 10.f);
		return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false
	}

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
	else PRINT_LOCAL(GetWorld(), "AttachToComponent(Holster) failed", FColor::Cyan, 10.f);
	
	return bIsAttached;
}

bool AC_GunBase::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser) return false;

	// 발사를 실패해도, 어쨋든 이 무기 사용자를 받아서 m_OwnerPlayer로 둔다
	// EquippedComponent에서 장착 시,
	m_OwnerPlayer = _WeaponUser;

	// 이거 그러면 현재는 재장전 모션과 사격 모션의 Priority가 동일한데, 사격모션보다 재장전 모션의 Priority가 더 높?
	// 사격 일련의 과정 확인해보니, 사격보다 재장전 모션 우선순위가 더 높긴한거 같은데
	if (m_bIsReloading) return false; 

	/* 모든 총기류 공통적으로 방아쇠 당길 수 있는 상황인지 검사할 항목 부모 클래스에서 일괄 구현 */
	if (m_OwnerPlayer->IsDead()) return false;
	if (m_bIsFiring || m_CurrentAmmo <= 0) return false; // 이미 PullTrigger를 한 상황, 또는 모든 Ammo를 소진한 경우
	
	// 달리기 상태에서 사격 불가
	if (m_OwnerPlayer && m_OwnerPlayer->GetPlayerMoveState() == EPlayerPoseState::Sprint) return false;

	// 각 총기 OnStartFire 각자 override 받아 continue
	// 나머지 사격처리에 필요한 검사 및 실질적인 첫 발사 처리가 제대로 처리 되었을 경우 m_bIsFiring 값 수정은 각 총기에서 구현해줄 것
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
	
	if (!m_OwnerPlayer)
	{
		UC_Util::Print("[AC_GunBase::Reload] : Received weaponUser nullptr", FColor::Cyan, 10.f);
		return false;
	}

	// 이미 최대 장탄수, 또는 이미 Reload 처리 중이라면 (플레이어 애니메이션 재생 여부에 따라 결정)
	if (m_CurrentAmmo >= m_MaxAmmo || m_bIsReloading) return false;

	// 이거 Reload 동작이 제대로 play를 할 수 있는 상황이 아니라면, 아예 재장전 처리를 하지 않아야 함 (로컬 본인이 따져야 함)
	const float PlayResultDuration = m_OwnerPlayer->PlayAnimMontage(m_PlayerReloadAnimation);
	if (PlayResultDuration <= 0.f) return false; // 현재 다른 동작에 의해 재장전 처리를 할 수 없는 상황 (Priority에 의해 막힌 상황)

	// 총기 Mesh 또한 자기자신의 환경에서 바로 재생 처리
	if (m_WeaponMesh && m_ReloadAnimation)
		m_WeaponMesh->PlayAnimation(m_ReloadAnimation, false);
	
	// 재장전 동작이 문제 없이 시작된 상황
	m_bIsReloading = true;
	
	// 사격 중이었다면 사격을 중단 처리
	ReleaseTrigger();

	// 행동만 똑같이 처리하도록 수정함 (자신의 장탄수는 자신의 장전 동작이 끝난 이후로 처리를 할 것)
	Server_PlayReloadEffects();
	return true;
}

void AC_GunBase::UpdateAmmoInfoHUDForDrawEnd()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled()) return;

	if (ItemLinkComp)
	{
		UC_Util::Print("GUN : ItemLinkComp Is Valid");
		
		FInventoryEntry* Entry = ItemLinkComp->GetItemEntryPtr();
		
		if (Entry)
		{
			UC_Util::Print("GUN :Entry Is Valid");
			
			UC_Util::Print(Entry->CurCount);
		}
		else
		{
			UC_Util::Print("GUN :Entry Is Nullptr");
		}
	}
	else
	{
		UC_Util::Print("GUN :ItemLinkComp Is nullptr");
		
	}
	
	if (UI_MANAGER(GetWorld()))
		UI_MANAGER(GetWorld())->GetMainHUDWidget()->ToggleAmmoInfoVisibility(true, m_FireMode, m_CurrentAmmo, m_MaxAmmo);
}

void AC_GunBase::ReleaseTrigger()
{
	m_bIsFiring = false;
}

FVector AC_GunBase::LineTraceDamage
(
	const FVector&	CameraStart,
	const FRotator& CameraRot,
	AActor*&		OutHitActor
)
{
	OutHitActor = nullptr;

	if (!m_WeaponMesh || !GetWorld() || !m_OwnerPlayer)
		return FVector::ZeroVector;

	const FVector CameraForward   = CameraRot.Vector();
	static const float TraceRange = 10000.0f;

	FCollisionQueryParams QueryParams{};
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(m_OwnerPlayer);

	const FVector CameraEnd = CameraStart + (CameraForward * TraceRange);
	FHitResult CameraHitResult{};

	const bool bCameraHit = GetWorld()->LineTraceSingleByChannel
	(
		CameraHitResult,
		CameraStart,
		CameraEnd,
		ECC_Visibility,
		QueryParams
	);

	/*if (bCameraHit)
	{
		DrawDebugLine(GetWorld(), CameraStart, CameraHitResult.ImpactPoint, FColor::Yellow, false, 5.f);
		DrawDebugSphere(GetWorld(), CameraHitResult.ImpactPoint, 5.f, 10, FColor::Green, false, 5.f);
	}
	else DrawDebugLine(GetWorld(), CameraStart, CameraEnd, FColor::Yellow, false, 5.f);*/

	const FVector TargetPoint = bCameraHit ? CameraHitResult.ImpactPoint : CameraEnd;
	const FVector MuzzleStart = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector ShootDirection = (TargetPoint - MuzzleStart).GetSafeNormal();

	if (m_SpreadAngle > 0.0f)
	{
		float ConeHalfAngleRad = FMath::DegreesToRadians(m_SpreadAngle);
		ShootDirection = FMath::VRandCone(ShootDirection, ConeHalfAngleRad);
	}

	FVector FinalMuzzleEnd = MuzzleStart + (ShootDirection * TraceRange);
	FHitResult MuzzleHitResult;

	const bool bMuzzleHit = GetWorld()->LineTraceSingleByChannel
	(
		MuzzleHitResult,
		MuzzleStart,
		FinalMuzzleEnd,
		ECC_Visibility,
		QueryParams
	);
	
	/*if (bMuzzleHit)
	{
		DrawDebugLine(GetWorld(), MuzzleStart, MuzzleHitResult.ImpactPoint, FColor::Red, false, 5.f);
		DrawDebugSphere(GetWorld(), MuzzleHitResult.ImpactPoint, 5.f, 10, FColor::Blue, false, 5.f);
	}
	else DrawDebugLine(GetWorld(), MuzzleStart, FinalMuzzleEnd, FColor::Red, false, 5.f);*/

	// 최종으로 맞은 Actor
	if (bMuzzleHit)
	{
		OutHitActor = MuzzleHitResult.GetActor();
		return MuzzleHitResult.ImpactPoint;
	}
	
	/*if (bCameraHit)
	{
		OutHitActor = CameraHitResult.GetActor();
		return CameraHitResult.ImpactPoint;
	}*/

	return FinalMuzzleEnd;
}

bool AC_GunBase::ExecuteFire()
{
	if (m_OwnerPlayer && m_OwnerPlayer->IsDead())
	{
		ReleaseTrigger();
		return false;
	}
	// 달리기 시 사격 중단
	if (m_OwnerPlayer  &&  m_OwnerPlayer->GetPlayerMoveState() == EPlayerPoseState::Sprint)
	{
		ReleaseTrigger();
		return false;
	}

	if (m_CurrentAmmo <= 0 || m_bIsReloading)
	{
		ReleaseTrigger();
		return false;
	}

	// AnimMontage에 의해 사격 모션이 끊기거나 제대로 재생처리가 이루어지지 않은 상황
	// Priority가 더 높은 동작이 수행되고 있다고 판단
	if (!PlayFireEffects())
	{
		ReleaseTrigger();
		return false;
	}

	// 실질적인 발사 처리가 이루어졌다 판단
	m_CurrentAmmo--;

	if (m_CurrentAmmo <= 0)
	{
		m_CurrentAmmo = 0;
		m_bIsFiring   = false;
	}

	UpdateAmmoUI();
	
	SpawnShellEject();

	AActor* HitActor = nullptr;
	FVector ImpactPoint = FVector::ZeroVector;

	if (m_OwnerPlayer)
	{
		FVector CameraLoc;
		FRotator CameraRot;

		if (APlayerController* PC = Cast<APlayerController>(m_OwnerPlayer->GetController()))
		{
			PC->GetPlayerViewPoint(CameraLoc, CameraRot);
		}
		else
		{
			m_OwnerPlayer->GetActorEyesViewPoint(CameraLoc, CameraRot);
		}

		ImpactPoint = LineTraceDamage(CameraLoc, CameraRot, HitActor);
	}

	Server_ExecuteFire(ImpactPoint, HitActor);
	return true;
}

void AC_GunBase::Server_ExecuteFire_Implementation(FVector_NetQuantize ImpactPoint, AActor* HitActor)
{

	if (HitActor && HitActor->IsA<AC_BasicEnemy>())
	{
		AController* InstigatorController = m_OwnerPlayer ? m_OwnerPlayer->GetController() : nullptr;
		UGameplayStatics::ApplyDamage(HitActor, m_Damage, InstigatorController, this, nullptr);
	}

	Multicast_PlayFireEffects(ImpactPoint);
}

void AC_GunBase::Server_PlayReloadEffects_Implementation()
{
	// 타인에게 Reload 재생처리 알림
	Multicast_PlayReloadEffects();
}

void AC_GunBase::OnSheathStart()
{
	// 사격 해제
	ReleaseTrigger();

	if (!m_OwnerPlayer) return;
	
	// Aim 카메라 및 UI 등 조준 관련 원위치
	if (m_OwnerPlayer->GetAimComponent())
		m_OwnerPlayer->GetAimComponent()->OnAimReleased();

	// 총기, 몽타주 재장전 애니메이션 해제 (만약 재장전 중이었다면)
	if (m_OwnerPlayer->GetMesh()->GetAnimInstance()->Montage_IsPlaying(m_PlayerReloadAnimation))
	{
		m_OwnerPlayer->StopAnimMontage(m_PlayerReloadAnimation);
		
	}

	// Sheath가 시작되면, 재장전을 하고있든말든 WeaponMesh의 Animation을 빼버리는 처리를 한다
	if (m_WeaponMesh && m_ReloadAnimation)
		m_WeaponMesh->SetAnimation(nullptr);

	m_bIsReloading = false;
	Server_CancelReload();
}

void AC_GunBase::Multicast_PlayAIFireEffects_Implementation(FVector_NetQuantize ImpactPoint)
{
	if (m_WeaponMesh && m_FireAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_FireAnimation, false);
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