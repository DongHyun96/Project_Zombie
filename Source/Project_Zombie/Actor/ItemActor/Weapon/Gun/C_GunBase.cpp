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
#include "Actor/ItemActor/Weapon/WeaponComponent/GunComponent/C_AIGunUsageComponent.h"

#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameModeAndManager/C_ItemManager.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

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

bool AC_GunBase::InitializeItemActor(const FWeaponData* InRawData)
{
	//return Super::InitializeItemActor(InRawData);
	
	const FGunData* GunData = static_cast<const FGunData*>(InRawData);
	
	if (!GunData)
	{
		UC_Util::Print("Failed Cast to const FGunData*", FColor::Red, 10.f);
		return false;
	}
	
	if (GunData->WeaponSkeletalMesh.IsValid() || !GunData->WeaponSkeletalMesh.IsNull())
	{
		USkeletalMesh* MeshAsset = GunData->WeaponSkeletalMesh.LoadSynchronous();
		if (m_WeaponMesh && MeshAsset)
		{
			m_WeaponMesh->SetSkeletalMesh(MeshAsset);
		}
	}
	else
	{
		UC_Util::Print("GunData->WeaponSkeletalMesh Is Not Valid", FColor::Red, 10.f);
		return false;
	}
	
	// 에셋 캐싱
	m_FireAnimation = GunData->FireAnimation.LoadSynchronous();
	m_ReloadAnimation = GunData->ReloadAnimation.LoadSynchronous();
	m_ShellMesh = GunData->ShellMesh.LoadSynchronous();
	m_PlayerReloadAnimation = GunData->PlayerReloadAnimation.LoadSynchronous();
	m_PlayerFireAnimation = GunData->PlayerFireAnimation.LoadSynchronous();
	
	// Base Stats 적용
	float BaseDamage = GunData->BaseDamage;
	int32 BaseMaxAmmo = GunData->MaxAmmo;
	m_FireRate = GunData->AttackRate;
	m_ShellEjectImpulse = GunData->ShellEjectImpulse;
	
	if (!ItemLinkComp)
	{
		UC_Util::Print("GunBase : Item Link Component Is Nullptr!", FColor::Red, 10.f);
	}
	
	// 동적 데이터 임시 처리.
	if (FInventoryEntry* EntryPtr = ItemLinkComp ? ItemLinkComp->GetItemEntryPtr() : nullptr)
	{
		const FGunCustomData* GunCustomData = EntryPtr->CustomData.GetPtr<FGunCustomData>();
		
		// 동적 데이터가 없으면 만들어서 넣어주기.
		if (!GunCustomData)
		{
			FGunCustomData TempCustomData{};
			EntryPtr->CustomData = FInstancedStruct::Make(TempCustomData);
		}
		
		GunCustomData = EntryPtr->CustomData.GetPtr<FGunCustomData>();
		
		m_Damage = BaseDamage + (GunCustomData->Upgrade_Damage * 5.0f);
		m_MaxAmmo = BaseMaxAmmo + (GunCustomData->Upgrade_MaxAmmo * 5);
		m_CurrentAmmo = GunCustomData->CurAmmo;
	}
	else
	{
		// Link가 무효한 상태일 때 기본값 처리
		m_Damage = BaseDamage;
		m_MaxAmmo = BaseMaxAmmo;
		m_CurrentAmmo = m_MaxAmmo;
	}
	
	//// 동적 데이터 임시 처리.
	//if (const FGunCustomData* GunCustomData = ItemLinkComp->GetItemEntryPtr()->CustomData.GetPtr<FGunCustomData>())
	//{
	//	m_Damage = BaseDamage + (GunCustomData->Upgrade_Damage * 5.0f);
	//	m_MaxAmmo = BaseMaxAmmo + (GunCustomData->Upgrade_MaxAmmo * 5);
	//	m_CurrentAmmo = GunCustomData->CurAmmo;
	//}
	//else
	//{
	//	// CustomData가 없는 초기 아이템 상태
	//	m_Damage = BaseDamage;
	//	m_MaxAmmo = BaseMaxAmmo;
	//	m_CurrentAmmo = m_MaxAmmo;
	//}
	
	return true;
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
	
	if (FInventoryEntry* EntryPtr = ItemLinkComp->GetItemEntryPtr())
	{
		if (FGunCustomData* GunCustomData = EntryPtr->CustomData.GetMutablePtr<FGunCustomData>())
		{
			GunCustomData->CurAmmo = m_CurrentAmmo;
			UC_Util::Print(GunCustomData->CurAmmo);
		}
	}

	// 현재 남은 장탄수 UI 업데이트
	if (AC_UIManager* UIManager = Cast<AC_UIManager>(GetWorld()->GetFirstPlayerController()->GetHUD()))
		UIManager->GetMainHUDWidget()->UpdateMagazineAmmoCount(m_CurrentAmmo);

	return true;
}

void AC_GunBase::SpawnShellEject()
{
	// 나이아가라 시스템, 탄피 메시, 무기 메시가 모두 유효할 때만 실행
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
		// Gun_init()에서 데이터 테이블로 로드해둔 m_ShellMesh를 나이아가라 변수로 넘김
		NiagaraComp->SetVariableStaticMesh(FName("ShellMesh"), m_ShellMesh);

		// 일반 총기는 1발
		NiagaraComp->SetIntParameter(FName("ShellCount"), 1);
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