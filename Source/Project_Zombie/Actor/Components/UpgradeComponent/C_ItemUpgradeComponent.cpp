// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Components/UpgradeComponent/C_ItemUpgradeComponent.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Actor/Components/C_EquippedComponent.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "Controller/C_BasicPlayerController.h"
#include "GameFramework/GameSession.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Utility/C_Util.h"

// Sets default values for this component's properties
UC_ItemUpgradeComponent::UC_ItemUpgradeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UC_ItemUpgradeComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UC_ItemUpgradeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UC_ItemUpgradeComponent::UpgradeItem(AC_BasicPlayer* InPlayer, int32 InItemIndex, EUpgradableStats TargetStat)
{
	UC_InvenComponent* InvenComp = InPlayer->GetInvenComponent();

	FInventoryEntry* Entry = InvenComp->GetSlotDataPtr(InItemIndex);

	PRINT_LOCAL(GetWorld(), "UpgradeItem", FColor::Blue, 5.f);

	if (!Entry->HasEquipmentData()) return;

	//Entry.CustomData.GetPtr

	FEquipmentCustomData* EquipmentData = Entry->GetEquipmentDataPtr();

	if (!EquipmentData) return;

	if (EquipmentData->GetStatGrade(TargetStat) >= MAX_GRADE)
	{
		AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(InPlayer->GetController());
	
		PC->SetIsUpgrading(false);
		
		if (PC->IsLocalPlayerController())
			PC->FinishItemUpgrade();
		else
			PC->Client_FinishItemUpgrade();
		
		return;
	}
	
	EquipmentData->AddStatGrade(TargetStat, 1);
	
	InvenComp->MarkSlotDirty(InItemIndex);

	// 장착 중인 아이템이면 AC_WeaponBase의 객체도 업데이트 해주어야 함.
	if (InItemIndex < static_cast<int32>(EWeaponSlot::None))
	{
		UC_EquippedComponent* EquipComp = InPlayer->GetEquippedComponent();

		AC_WeaponBase* CurWeapon = EquipComp->GetSlotWeapon(static_cast<EWeaponSlot>(InItemIndex));

		UC_ItemManager* ItemManager = InPlayer->GetGameInstance()->GetSubsystem<UC_ItemManager>();

		if (!ItemManager) return;

		const FWeaponData* WeaponData = nullptr;

		WeaponData = ItemManager->GetWeaponData(Entry->ItemRowName);

		if (WeaponData)
		{
			// TODO : 서버에서 이 아이템의 정보를 알 필요 없다면 안해도 될 수 도?
			CurWeapon->InitializeItemData(WeaponData);
			
			// TODO : 클라에서 InitializeItemData함수를 호출해서 클라가 자신의 강화를 착용중인 아이템에 적용 해야 하는데.
			// 클라에서 InitializeItemData를 호출시키는 함수를 하나 만들거나, 강제로 무기를 재장착 하는 방법이 있음.
			if (InPlayer->IsLocallyControlled())
				EquipComp->UpdateWeaponData(static_cast<EWeaponSlot>(InItemIndex), Entry->ItemRowName);
			else
				EquipComp->Client_UpdateWeaponData(static_cast<EWeaponSlot>(InItemIndex), Entry->ItemRowName);
		}
	}
	//GetPlayerControllerFromNetId()
	AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(InPlayer->GetController());
	
	PC->SetIsUpgrading(false);
	
	// TODO : UpdgradeWidget UpdateWidget 호출을 여기서 해주어야 강화 후 위젯 갱신이 가능.
	// 현재 여기 위치는 서버이기 때문에 클라와 서버 나눠서 업데이트 해주어야 함. 
	
	if (PC->IsLocalPlayerController())
		PC->FinishItemUpgrade();
	else
		PC->Client_FinishItemUpgrade();
}


