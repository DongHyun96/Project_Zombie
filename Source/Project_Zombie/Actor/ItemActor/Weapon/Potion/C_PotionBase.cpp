// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ItemActor/Weapon/Potion/C_PotionBase.h"

#include "GlobalData.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_EquippedComponent.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Actor/Components/ItemLinkComponent/C_ItemLinkComponent.h"
#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/AssetManager.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Net/UnrealNetwork.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/MainHUD/PlayerStatHUD/C_PlayerStatWidget.h"
#include "Utility/C_Util.h"


// 투척류와 같은 소캣 사용.
const FName AC_PotionBase::s_HolsterSocketName = TEXT("ThrowableHolsterSocket");

AC_PotionBase::AC_PotionBase()
{
	PrimaryActorTick.bCanEverTick = false; // Tick 필요 없으면 끄기.
	
	capsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = capsuleComponent;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	
	
}

void AC_PotionBase::OnAction()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled())
		return;
	
	UC_StatComponentBase* StatComp = m_OwnerPlayer->GetStatComponent();
	
	float MaxHP = StatComp->GetStat(StatName::MaxHP);
	
	float RecoverHPValue = MaxHP * Potion_value;
	
	// HP의 서버 동기화까지 함께 진행되는 함수.
	m_OwnerPlayer->GetStatComponent()->IncreaseCurHP(RecoverHPValue);
	
	// Count를 줄여줘야함.
	Server_DecreaseCurCount();
}

bool AC_PotionBase::InitializeItemActor(const FWeaponData* InRawData)
{
	const FPotionData* PotionData = static_cast<const FPotionData*>(InRawData);
	
	if (!PotionData)
	{
		UC_Util::Print("Failed Cast to const FGunData*", FColor::Red, 10.f);
		return false;
	}
	
	InitializeItemData(InRawData);
	
	LoadAsyncAssets(PotionData);
	
	return true;
}

void AC_PotionBase::InitializeItemData(const FWeaponData* InRawData)
{
	const FPotionData* PotionData = static_cast<const FPotionData*>(InRawData);
	
	if (!PotionData)
	{
		UC_Util::Print("Failed Cast to const FGunData*", FColor::Red, 10.f);
		return;
	}
	
	float BaseValue = PotionData->Value;
	
	if (!ItemLinkComp)
	{
		UC_Util::Print("Potion : Item Link Component Is Nullptr!", FColor::Red, 10.f);
	}
	
	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	if (!ItemManager) return;
	
	const FWeaponUpgradeData* UpgradeData = ItemManager->GetWeaponUpgradeData(m_WeaponRowName);
	
	if (!UpgradeData) return;
	
	//if (!ItemLinkComp->IsLinkValid()) return;
	
	
	
	// 동적 데이터(CustomData) 처리
	if (FInventoryEntry* EntryPtr = ItemLinkComp->GetItemEntryPtr())
	{
		// 1. 없으면 데이터 안전하게 생성
		FUpgradableData* CustomData = EntryPtr->GetOrCreateEquipmentData();
		
		uint8 ValueGrade = 0;
		
		if (CustomData)
			ValueGrade = CustomData->GetStatGrade(EUpgradableStats::HPRecovery);
		
		Potion_value = BaseValue + ValueGrade * UpgradeData->GradePerValue[EUpgradableStats::HPRecovery];
		//LeftTotalCount = EntryPtr->CurCount;
	}
	else
	{
		Potion_value = BaseValue;
	}
}

void AC_PotionBase::LoadAsyncAssets(const FWeaponData* InRawData)
{
	const FPotionData* PotionData = static_cast<const FPotionData*>(InRawData);
	
	if (!PotionData)
	{
		UC_Util::Print("Failed Cast to const FGunData*", FColor::Red, 10.f);
		return;
	}
	
	CancelAsyncLoad();
	
	// 비동기로 로드할 SoftObjectPath 목록 수집
	TArray<FSoftObjectPath> AssetsToLoad;
	
	if (!PotionData->WeaponStaticMesh.IsNull()) AssetsToLoad.Add(PotionData->WeaponStaticMesh.ToSoftObjectPath());
	if (!PotionData->UsingMontage.IsNull()) AssetsToLoad.Add(PotionData->UsingMontage.ToSoftObjectPath());
	
	if (AssetsToLoad.Num() > 0)
	{
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		
		TSoftObjectPtr<UStaticMesh> SoftMesh = PotionData->WeaponStaticMesh;
		TSoftObjectPtr<UAnimMontage> SoftUsingMontage = PotionData->UsingMontage;
		
		m_AsyncLoadHandle = Streamable.RequestAsyncLoad(
			AssetsToLoad,
			FStreamableDelegate::CreateWeakLambda(this, [this, SoftMesh, SoftUsingMontage]()
			{
				if (SoftMesh.IsNull() && StaticMeshComponent)
					StaticMeshComponent->SetStaticMesh(SoftMesh.Get());
				
				m_UsingAnimation = SoftUsingMontage.Get();
			})
		);
	}
}


bool AC_PotionBase::OnStartFire(class AC_BasicPlayer* _WeaponUser)
{
	
	// TODO : 아이템 사용 시작, UsingMontage 실행 해야함.
	/*
	 * 1. 아이템 사용 시작
	 * 2. 로컬에서 우선 사용 가능한지 판정.
	 * 3. 사용 가능
	 * 4. 로컬에서 우선 UsingMontage실행
	 * 5. 서버에 UsingMontage 실행과 아이템 사용을 요청.
	 * 6. 서버에서 아이템 사용을 멀티캐스트(모두가 봐야 하니까)
	 */

	if (!_WeaponUser)
		return false;
	
	// 풀피 이상이면 사용 불가.
	if (_WeaponUser->GetStatComponent()->GetCurHPRatio() >= 1.f) return false;
	
	if (_WeaponUser->GetMesh()->GetAnimInstance()->Montage_IsPlaying(m_UsingAnimation))
	{
		return false;
	}
		
	// 투척류 애니메이션 재생
	//PlayThrowMontageSynced(StartSectionName);
	
	if (!m_OwnerPlayer->IsLocallyControlled())
		return false;
	
	PlayUsingMontageSynced();
	
	return true;
}

bool AC_PotionBase::AttachToHolster(USceneComponent* _ParentMesh)
{
	// TODO : 수류탄 처럼 허리춤에?
	if (!_ParentMesh) return false;
	if (!Cast<AC_BasicPlayer>(_ParentMesh->GetOwner())) return false; // 무기집에 붙이려는 Actor가 Player형이 아닌 경우
	
	SetActorHiddenInGame(true);
	
	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
		s_HolsterSocketName
	);
	
	if (bIsAttached)
		m_OwnerPlayer = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	
	return bIsAttached;
}

bool AC_PotionBase::AttachToHand(USceneComponent* _ParentMesh)
{
	// TODO : 수류탄처럼 손에 쥐어 주어야 함.
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false
	
	SetActorHiddenInGame(false);
	
		const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		m_HandSocketName
	);
	
	if (bIsAttached)
	{
		Player->SetHandState(EHandState::WeaponThrowable);
		UpdateAmmoInfoHUDForDrawEnd();
	}
	
	return bIsAttached;
}

void AC_PotionBase::SetAmmoUIInfo(FAmmoUIInfo& _AmmoUIInfo)
{
	_AmmoUIInfo.Visible            = true;
	_AmmoUIInfo.FireMode           = EFireMode::Single;
	_AmmoUIInfo.MagazineAmmo       = 1;
	
	int32 Count = 1;
	
	if (FInventoryEntry* Entry = ItemLinkComp->GetItemEntryPtr())
		Count = Entry->CurCount;
	
	_AmmoUIInfo.LeftAmmoTotalCount = Count;
}

void AC_PotionBase::UpdateAmmoInfoHUDForDrawEnd()
{
	if (!m_OwnerPlayer || !m_OwnerPlayer->IsLocallyControlled()) return;
	
	int32 Count = 1;
	
	if (FInventoryEntry* Entry = ItemLinkComp->GetItemEntryPtr())
		Count = Entry->CurCount;
	
	UI_MANAGER(GetWorld())->GetMainHUDWidget()->ToggleAmmoInfoVisibility(true, EFireMode::Single, 1, Count);
}

void AC_PotionBase::OnRep_UpdateAmmoWidget()
{
	UpdateAmmoInfoHUDForDrawEnd();
}

void AC_PotionBase::PlayUsingMontageSynced()
{
	m_OwnerPlayer->PlayAnimMontage(m_UsingAnimation, 1.f);
	
	// 서버에서 실행 중이면, 멀티캐스트로 다른 클라이언트에게도 재생
	if (HasAuthority())
	{
		Multicast_PlayUsingMontage();
		return;
	}

	// 서버에서 실행 중이 아니면, 서버에 재생 요청
	Server_PlayUsingMontage();
}

void AC_PotionBase::Server_PlayUsingMontage_Implementation()
{
	Multicast_PlayUsingMontage();
}

void AC_PotionBase::Multicast_PlayUsingMontage_Implementation()
{
	if (!m_OwnerPlayer || !m_UsingAnimation)
		return;

	// 이미 로컬에서 재생한 경우, 중복 재생 방지
	if (m_OwnerPlayer->IsLocallyControlled())
		return;
	
	m_OwnerPlayer->PlayAnimMontage(m_UsingAnimation, 1.f);
}

void AC_PotionBase::Server_DecreaseCurCount_Implementation()
{
	if (ItemLinkComp)
	{
		if (FInventoryEntry* SlotEntry = ItemLinkComp->GetItemEntryPtr())
		{
			UC_Util::Print("Potion Decrease");
			
			--SlotEntry->CurCount;
			//LeftTotalCount = SlotEntry->CurCount;
			int32 Idx = ItemLinkComp->GetSlotIndex();
			if (SlotEntry->CurCount <= 0)
			{
				//LeftTotalCount = 0;
				SlotEntry->Clear();
				SlotEntry->SlotIndex = Idx;
			}
			// TODO : 직후에 인벤토리를 업데이트 해주어야 함.

			
			// 서버에서 사용하면 이걸 써야 할 걸?
			
			ItemLinkComp->GetOwningInvenComp()->MarkSlotDirty(Idx);		
			
			ItemLinkComp->GetOwningInvenComp()->OnInventorySlotChanged.Broadcast(Idx, *SlotEntry);
		}
	}
}