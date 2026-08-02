// Fill out your copyright notice in the Description page of Project Settings.


#include "C_WeaponBase.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
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
	if (m_WeaponRowName.IsNone()) return;

	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	if (!ItemManager) return;

	const FWeaponData* WeaponData = nullptr;
	
	WeaponData = ItemManager->GetWeaponData(m_WeaponRowName);

	if (WeaponData)
	{
		PRINT_LOCAL(GetWorld(), "LoadAssets :::::::::::::::::::::::::::", FColor::Cyan, 5.f);

		LoadAsyncAssets(WeaponData);
	}
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

