// Fill out your copyright notice in the Description page of Project Settings.


#include "C_MeleeWeaponBase.h"

#include "../WeaponComponent/MeleeComponent/C_MeleeDataTableComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/ItemLinkComponent/C_ItemLinkComponent.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Utility/C_Util.h"


AC_MeleeWeaponBase::AC_MeleeWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	m_WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = m_WeaponMesh;

	m_DataCom = CreateDefaultSubobject<UC_MeleeDataTableComponent>(TEXT("DataComponent"));
}

void AC_MeleeWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	Melee_init();
}

void AC_MeleeWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_MeleeWeaponBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	Melee_init();
}

bool AC_MeleeWeaponBase::InitializeItemActor(const FWeaponData* InRawData)
{
	//return Super::InitializeItemActor(InRawData);
	
	const FMeleeData* MeleeData = static_cast<const FMeleeData*>(InRawData);
	
	if (!MeleeData)
	{
		UC_Util::Print("Failed Cast to const FMeleeData*", FColor::Red, 10.f);
		return false;
	}
	
	if (UStaticMesh* WeaponMeshAsset = (MeleeData->WeaponStaticMesh.LoadSynchronous()))
	{
		if (m_WeaponMesh)
		{
			m_WeaponMesh->SetStaticMesh(WeaponMeshAsset);
		}
	}
	else
	{
		// 테이블에 에셋이 없을 때만 경고
		UE_LOG(LogTemp, Warning, TEXT("데이터 테이블에 WeaponStaticMesh가 없음!"));
	}
	
	// 에셋 캐싱
	m_PlayerAttackAnimation = MeleeData->PlayerAttackAnimation.LoadSynchronous();
	
	// Base Stats 적용
	float BaseDamage = MeleeData->BaseDamage;
	//int32 BaseMaxAmmo = MeleeData->MaxAmmo;
	//m_FireRate = GunData->AttackRate;
	//m_ShellEjectImpulse = GunData->ShellEjectImpulse;
	
	if (!ItemLinkComp)
	{
		UC_Util::Print("MeleeBase : Item Link Component Is Nullptr!", FColor::Red, 10.f);
	}
	
	// 동적 데이터 임시 처리.
	if (FInventoryEntry* EntryPtr = ItemLinkComp ? ItemLinkComp->GetItemEntryPtr() : nullptr)
	{
		if (const FGunCustomData* GunCustomData = EntryPtr->CustomData.GetPtr<FGunCustomData>())
		{
			m_Damage = BaseDamage + (GunCustomData->Upgrade_Damage * 5.0f);
		}
		else
		{
			// CustomData가 없는 초기 아이템 상태
			m_Damage = BaseDamage;
		}
	}
	
	return true;
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

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		m_HolsterSocketName
	);
	
	return bIsAttached;
}

bool AC_MeleeWeaponBase::AttachToHand(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false

	m_OwnerPlayer = Player;

	// Main HUD MeleeWeapon 종류로 초기화
	// TODO : 각 MeleeWeapon에 맞는 이미지 아이콘(?) 표시해주면 좋을 듯 (일단은 AmmoInfo쪽 정보 감추는 처리로 함)
	if (AC_UIManager* UIManager = Cast<AC_UIManager>(Player->GetController<APlayerController>()->GetHUD()))
		UIManager->GetMainHUDWidget()->ToggleAmmoInfoVisibility(false);

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		m_HandSocketName
	);
	
	if (bIsAttached)
    		Player->SetHandState(EHandState::WeaponMelee);
	
	return bIsAttached;
}

void AC_MeleeWeaponBase::Melee_init()
{
	if (!m_DataCom) return;

	// 외형(Mesh) 로드
	if (UStaticMesh* WeaponMeshAsset = Cast<UStaticMesh>(m_DataCom->GetAssetData("WeaponStaticMesh").LoadSynchronous()))
	{
		if (m_WeaponMesh)
		{
			m_WeaponMesh->SetStaticMesh(WeaponMeshAsset);
		}
	}
	else
	{
		// 테이블에 에셋이 없을 때만 경고
		UE_LOG(LogTemp, Warning, TEXT("데이터 테이블에 WeaponStaticMesh가 없음!"));
	}

	// 에디터 뷰포트에서 총기를 드래그해 움직일 때는 아래 '무거운 로직/수치 계산'을 패스
	// HasActorBegunPlay()는 실제 게임 플레이 버튼을 눌렀을 때만 true
	if (!HasActorBegunPlay())
	{
		return;
	}

	m_Damage = m_DataCom->GetData("BaseDamage");

	m_PlayerAttackAnimation = Cast<UAnimMontage>(m_DataCom->GetAssetData("PlayerAttackAnimation").LoadSynchronous());

	if (!m_PlayerAttackAnimation) { UE_LOG(LogTemp, Warning, TEXT("PlayerAttackAnimation 로드 실패")); }
}

void AC_MeleeWeaponBase::Attack(AC_BasicPlayer* _WeaponUser)
{
	if (!_WeaponUser || !m_PlayerAttackAnimation) return;

	FVector CurSockPos = m_WeaponMesh->GetSocketLocation(TEXT("HitBoxSock"));

	_WeaponUser->PlayAnimMontage(m_PlayerAttackAnimation);

}
