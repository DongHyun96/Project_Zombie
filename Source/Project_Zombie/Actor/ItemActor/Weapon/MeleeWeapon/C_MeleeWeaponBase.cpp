// Fill out your copyright notice in the Description page of Project Settings.


#include "C_MeleeWeaponBase.h"

#include "../WeaponComponent/MeleeComponent/C_MeleeDataTableComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_EquippedComponent.h"
#include "Actor/Components/ItemLinkComponent/C_ItemLinkComponent.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Utility/C_Util.h"

AC_MeleeWeaponBase::AC_MeleeWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	m_WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = m_WeaponMesh;

	bReplicates = true;
	SetReplicateMovement(true);
}

void AC_MeleeWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	//Melee_init();
}

void AC_MeleeWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AC_MeleeWeaponBase::InitializeItemActor(const FWeaponData* InRawData)
{
	//return Super::InitializeItemActor(InRawData);
	
	const FMeleeData* MeleeData = static_cast<const FMeleeData*>(InRawData);
	
	InitializeItemData(InRawData);

	LoadAsyncAssets(InRawData);
	
	return true;
}

void AC_MeleeWeaponBase::InitializeItemData(const FWeaponData* InRawData)
{
	const FMeleeData* MeleeData = static_cast<const FMeleeData*>(InRawData);

	float BaseDamage = MeleeData->BaseDamage;

	if (!ItemLinkComp)
	{
		UC_Util::Print("MeleeBase : Item Link Component Is Nullptr!", FColor::Red, 10.f);
		return;
	}

	// 이 부분을 C_WeaponBase에서 ItemInit함수로 따로 빼서 만들어야 나중에 강화 할 때 이 함수만 불러서 강화된 스탯을 업데이트 할 수 있지 않을까?
	if (FInventoryEntry* EntryPtr = ItemLinkComp ? ItemLinkComp->GetItemEntryPtr() : nullptr)
	{
		// 1. 없으면 데이터 안전하게 생성
		FUpgradableData* CustomData = EntryPtr->GetOrCreateEquipmentData();

		// 2. Grade(단계) 가져오기
		int32 DamageGrade = CustomData->GetStatGrade(EUpgradableStats::AttackPower);

		// TODO : 나중에 근접 무기의 공격속도 멤버 변수를 추가하면 사용할 것
		//int32 AttackRatio = CustomData->GetStatGrade(EUpgradableStats::FireRate);

		// 3. 최종 스탯 계산: BaseStat + (Grade * DataAsset의 레벨당 증가량)
		m_Damage = MeleeData->BaseDamage + (DamageGrade * MeleeData->DamagePerUpgradeLevel);


		// TODO : 나중에 근접 무기의 공격속도 멤버 변수를 추가하면 사용할 것
		//m_AttackRatio = MeleeData->AttackRate + (DamageGrade * MeleeData->AttackRatePerUpgradeLevel);
	}
	else
	{
		m_Damage = BaseDamage;
	}
}


void AC_MeleeWeaponBase::LoadAsyncAssets(const FWeaponData* InRawData)
{
	const FMeleeData* MeleeData = static_cast<const FMeleeData*>(InRawData);
	if (!MeleeData)
	{
		UC_Util::Print("Failed Cast to const FMeleeData*", FColor::Red, 10.f);
		return;
	}

	// 기존 로딩 핸들 취소 및 정리
	CancelAsyncLoad();

	// 비동기로 로드할 SoftObjectPath 목록 수집
	TArray<FSoftObjectPath> AssetsToLoad;

	if (!MeleeData->WeaponStaticMesh.IsNull())        AssetsToLoad.Add(MeleeData->WeaponStaticMesh.ToSoftObjectPath());
	if (!MeleeData->PlayerAttackAnimation.IsNull())  AssetsToLoad.Add(MeleeData->PlayerAttackAnimation.ToSoftObjectPath());

	if (AssetsToLoad.Num() > 0)
	{
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

		// 람다 안전 캡처용 SoftPointer 복사
		TSoftObjectPtr<UStaticMesh>  SoftMesh       = MeleeData->WeaponStaticMesh;
		TSoftObjectPtr<UAnimMontage> SoftAttackAnim = MeleeData->PlayerAttackAnimation; // 타입에 맞게 UAnimSequence/UAnimMontage 지정

		m_AsyncLoadHandle = Streamable.RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateLambda([
			this,
			SoftMesh,
			SoftAttackAnim
		]()
		{
			if (!IsValid(this)) return;

			// 1. 스태틱 메쉬 설정
			if (SoftMesh.IsValid())
			{
				if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(m_WeaponMesh))
				{
					StaticMeshComp->SetStaticMesh(SoftMesh.Get());
				}
			}

			// 2. 애니메이션 에셋 캐싱
			if (SoftAttackAnim.IsValid())
			{
				m_PlayerAttackAnimation = SoftAttackAnim.Get();
			}

			UC_Util::Print("Melee Weapon Assets Async Loaded Successfully!", FColor::Green, 5.f);

			// 로딩 완료 후 핸들 정리
			if (m_AsyncLoadHandle.IsValid())
			{
				m_AsyncLoadHandle.Reset();
			}
		}));
	}
}

void AC_MeleeWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool AC_MeleeWeaponBase::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	// TODO : LMB 첫 눌렸을 시, 동작 처리
	if (nullptr == _WeaponUser)
	{
		return false;
	}
	else
	{
		Attack(_WeaponUser);
		return true;
	}
}

bool AC_MeleeWeaponBase::AttachToHolster(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false

	SetOwner(Player);

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		m_HolsterSocketName
	);
	
	if (bIsAttached) m_OwnerPlayer = Player;
	
	return bIsAttached;
}

bool AC_MeleeWeaponBase::AttachToHand(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false

	PRINT_LOCAL(GetWorld(), "Melee - AttachingToHand", FColor::Red, 10.f);
	
	SetOwner(Player);
	m_OwnerPlayer = Player;

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		m_HandSocketName
	);
	
	if (bIsAttached)
	{
    	Player->SetHandState(EHandState::WeaponMelee);
		UpdateAmmoInfoHUDForDrawEnd();
	}
	
	return bIsAttached;
}

void AC_MeleeWeaponBase::Attack(AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser) return;

	PlayAttackMotion(_WeaponUser);
}

void AC_MeleeWeaponBase::PlayAttackMotion(AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser || !m_PlayerAttackAnimation) return;

	UAnimInstance* AnimInstance = _WeaponUser->GetMesh() ? _WeaponUser->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance) return;

	if (!AnimInstance->Montage_IsPlaying(m_PlayerAttackAnimation))
	{
		m_bSaveCombo = false;
		m_HitActors.Empty();
		m_PrevHitBoxSockPos = m_WeaponMesh->GetSocketLocation(TEXT("HitBoxSock"));

		if (_WeaponUser->IsLocallyControlled() && !HasAuthority())
		{
			AnimInstance->Montage_Play(m_PlayerAttackAnimation);
			Server_ReqMeleeCombo();
		}
		else if (HasAuthority())
		{
			Server_ReqMeleeCombo_Implementation();
		}
	}
	else
	{
		m_bSaveCombo = true;
	}
}

void AC_MeleeWeaponBase::Server_ReqMeleeCombo_Implementation()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->GetMesh()) return;

	UAnimInstance* AnimInstance = m_OwnerPlayer->GetMesh()->GetAnimInstance();
	if (!AnimInstance || !m_PlayerAttackAnimation) return;

	FName TargetSection = NAME_None;

	if (!AnimInstance->Montage_IsPlaying(m_PlayerAttackAnimation))
	{
		AnimInstance->Montage_Play(m_PlayerAttackAnimation);
	}
	else
	{
		TargetSection = FName("Combo");
		AnimInstance->Montage_JumpToSection(TargetSection, m_PlayerAttackAnimation);
	}

	Multicast_PlayAttackMotion(TargetSection);
}

void AC_MeleeWeaponBase::Multicast_PlayAttackMotion_Implementation(FName SectionName)
{
	// 서버 본인이나 공격을 시도한 로컬 플레이어는 이미 재생했으므로 중복 재생 방지
	if (!m_OwnerPlayer || m_OwnerPlayer->IsLocallyControlled()) return;

	UAnimInstance* AnimInstance = nullptr;
		
	if (m_OwnerPlayer->GetMesh())
		AnimInstance = m_OwnerPlayer->GetMesh()->GetAnimInstance();

	if (!AnimInstance || !m_PlayerAttackAnimation) return;

	if (SectionName == FName("Combo"))
	{
		AnimInstance->Montage_JumpToSection(FName("Combo"), m_PlayerAttackAnimation);
	}
	else
	{
		AnimInstance->Montage_Play(m_PlayerAttackAnimation);
	}
}

void AC_MeleeWeaponBase::Server_ApplyHitDamage_Implementation(AActor* HitActor, float Damage, FVector ImpactPoint, FVector ImpactNormal)
{
	if (!HasAuthority() || !HitActor) return;

	AController* InstigatorController = nullptr;
	if (m_OwnerPlayer)
	{
		InstigatorController = m_OwnerPlayer->GetController();
		if (!InstigatorController)
		{
			InstigatorController = m_OwnerPlayer->GetInstigatorController();
		}
	}
	float ActualDamage = UGameplayStatics::ApplyDamage(
		HitActor,
		Damage,
		InstigatorController,
		this,
		UDamageType::StaticClass()
	);

	Multicast_PlayHitEffect(ImpactPoint, ImpactNormal);
}

void AC_MeleeWeaponBase::Multicast_PlayHitEffect_Implementation(FVector ImpactPoint, FVector ImpactNormal)
{
	if (m_OwnerPlayer && m_OwnerPlayer->IsLocallyControlled()) return;

	if (HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			HitEffect,
			ImpactPoint,
			ImpactNormal.Rotation()
		);
	}

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			HitSound,
			ImpactPoint
		);
	}
}

void AC_MeleeWeaponBase::HitBoxCheck()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled()) return;

	FVector CurSockPos = m_WeaponMesh->GetSocketLocation(TEXT("HitBoxSock"));

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner()); // 공격자 본인 제외

	FCollisionShape ColShape = FCollisionShape::MakeSphere(50.f);

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		m_PrevHitBoxSockPos,
		CurSockPos,
		FQuat::Identity,
		ECC_Visibility,
		ColShape,
		Params
	);

	if (bHit)
	{
		for (const auto& Result : HitResults)
		{
			AActor* HitActor = Result.GetActor();
			if (HitActor && !m_HitActors.Contains(HitActor))
			{
				m_HitActors.Add(HitActor);

				if (HitEffect)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						GetWorld(), HitEffect, Result.ImpactPoint, Result.ImpactNormal.Rotation()
					);
				}

				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, Result.ImpactPoint);
				}

				Server_ApplyHitDamage(HitActor, m_Damage, Result.ImpactPoint, Result.ImpactNormal);
			}
		}
	}

	m_PrevHitBoxSockPos = CurSockPos;
}


void AC_MeleeWeaponBase::MeleeCombo()
{
	if (m_bSaveCombo)
	{
		m_bSaveCombo = false;

		if (!m_OwnerPlayer) return;

		m_HitActors.Empty();

		// 로컬 실행
		UAnimInstance* AnimInstance = m_OwnerPlayer->GetMesh()->GetAnimInstance();
		if (AnimInstance && m_PlayerAttackAnimation)
		{
			AnimInstance->Montage_JumpToSection(FName("Combo"), m_PlayerAttackAnimation);
		}

		if (m_OwnerPlayer->IsLocallyControlled() && !HasAuthority())
		{
			Server_ReqMeleeCombo();
		}
		else if (HasAuthority())
		{
			Server_ReqMeleeCombo_Implementation();
		}
	}
}

void AC_MeleeWeaponBase::UpdateAmmoInfoHUDForDrawEnd()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled()) return;
	
	// MeleeWeapon의 경우 띄워줄 AmmoInfo 정보 필요 없음
	UI_MANAGER(GetWorld())->GetMainHUDWidget()->ToggleAmmoInfoVisibility(false);
}

void AC_MeleeWeaponBase::SetAmmoUIInfo(FAmmoUIInfo& _AmmoUIInfo)
{
	_AmmoUIInfo.Visible = false;
}

