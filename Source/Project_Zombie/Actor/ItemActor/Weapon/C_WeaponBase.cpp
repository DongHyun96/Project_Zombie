
// Fill out your copyright notice in the Description page of Project Settings.


#include "C_WeaponBase.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Actor/Components/ItemLinkComponent/C_ItemLinkComponent.h"

#include "Engine/StreamableManager.h"
#include "GameModeAndManager/C_ItemManager.h"

#include "GameModeAndManager/C_UIManager.h"
#include "Net/UnrealNetwork.h"
#include "UI/MainHUD/C_GameMainHUD.h"


AC_WeaponBase::AC_WeaponBase()
{
	PrimaryActorTick.bCanEverTick = true; // Tick 필요 없으면 끄기.

	SetReplicates(true);
	
	ItemLinkComp = CreateDefaultSubobject<UC_ItemLinkComponent>(TEXT("ItemLinkComp"));
}

void AC_WeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
	m_InitialRelativeTransform = GetRootComponent()->GetRelativeTransform();
}

void AC_WeaponBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 액터가 파괴될 때 비동기 로딩이 아직 진행 중이라면 취소 및 핸들 정리
	CancelAsyncLoad();

	Super::EndPlay(EndPlayReason);
}

void AC_WeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_WeaponBase::OnRep_WeaponRowName()
{
	//ClientInitializeWeapon();
}

void AC_WeaponBase::OnRep_OwnerPlayer()
{
	//ClientInitializeWeapon();
}

void AC_WeaponBase::Multi_InitItemActor_Implementation(UC_InvenComponent* InInvenComp, int32 Idx)
{
	if (!InInvenComp || Idx == -1) return;
	
	FInventoryEntry Entry = InInvenComp->GetItemAt(Idx);

	if (Entry.IsEmpty()) return;
	
	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	if (!ItemManager) return;
	
	const FWeaponData* WeaponData = ItemManager->GetWeaponData(Entry.ItemRowName);
	
	if (!WeaponData) return; 
	
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(InInvenComp->GetOwner());
	
	if (!Player) return;
	
	if (Player->IsLocallyControlled())
	{
		if (ItemLinkComp)
		{
			ItemLinkComp->InitializeLink(InInvenComp, static_cast<int32>(WeaponData->WeaponType));
		}
           
		InitializeItemData(WeaponData);
		LoadAsyncAssets(WeaponData);
		
	}
	else
	{
		LoadAsyncAssets(WeaponData);
	}
}

void AC_WeaponBase::ClientInitializeWeapon()
{
	// [방어막 1] 둘 중 하나라도 아직 네트워크로 안 왔다면 패스합니다.
	if (m_WeaponRowName.IsNone() || !m_OwnerPlayer) return;

	// [방어막 2] 내 무기일 때만 로컬 데미지 계산 및 인벤토리 링크 생성
	if (m_OwnerPlayer->IsLocallyControlled())
	{
		UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
		if (!ItemManager) return;

		const FWeaponData* WeaponData = ItemManager->GetWeaponData(m_WeaponRowName);
		if (WeaponData)
		{
			UC_InvenComponent* OwnerInvenComp = m_OwnerPlayer->GetInvenComponent();
			if (!OwnerInvenComp) return;
            
			if (ItemLinkComp)
			{
				ItemLinkComp->InitializeLink(OwnerInvenComp, static_cast<int32>(WeaponData->WeaponType));
			}
            
			InitializeItemData(WeaponData);
			LoadAsyncAssets(WeaponData);
		}
	}
	else
	{
		// 다른 사람의 무기라면 화면에 보이기 위한 최소한의 시각적 초기화(메시 에셋 로드 등)만 수행
		UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
		if (ItemManager)
		{
			const FWeaponData* WeaponData = ItemManager->GetWeaponData(m_WeaponRowName);
			if (WeaponData)
			{
				// 타인 화면용 초기화 (인벤토리 링크 제외)
				InitializeItemData(WeaponData);
				LoadAsyncAssets(WeaponData);
			}
		}
	}
	
	/*if (m_WeaponRowName.IsNone()) return;

	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	if (!ItemManager) return;

	const FWeaponData* WeaponData = nullptr;
	
	WeaponData = ItemManager->GetWeaponData(m_WeaponRowName);

	if (WeaponData)
	{
		// 원래는 서버만 알게 할려고 했는데 현재 구조는 로컬에서 데미지 계산을 하고 최종값만 보내는 형식이라 필요해 졌음.
		UC_InvenComponent* OwnerInvenComp = m_OwnerPlayer->GetInvenComponent();
		
		if (!OwnerInvenComp) return;
		
		ItemLinkComp->InitializeLink(OwnerInvenComp, static_cast<int32>(WeaponData->WeaponType));
		
		InitializeItemData(WeaponData);
		LoadAsyncAssets(WeaponData);
	}*/
}

void AC_WeaponBase::CancelAsyncLoad()
{
	// 기존에 진행 중이던 비동기 로딩 요청이 남아있다면 먼저 취소/해제
	if (m_AsyncLoadHandle.IsValid())
	{
		// 로딩 요청을 취소하고 엔진 메모리에서 핸들 참조를 해제합니다.
		m_AsyncLoadHandle->CancelHandle();
		m_AsyncLoadHandle.Reset();
	}
}



void AC_WeaponBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 리플리케이트 하고싶은 멤버를 등록 여기서
	DOREPLIFETIME(AC_WeaponBase, m_OwnerPlayer);
	DOREPLIFETIME(AC_WeaponBase, m_WeaponRowName);
}

