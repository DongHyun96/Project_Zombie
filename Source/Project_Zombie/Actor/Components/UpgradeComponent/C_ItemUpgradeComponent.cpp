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
    if (!InPlayer) return;

    PRINT_LOCAL(GetWorld(), "UpgradeItem", FColor::Blue, 5.f);

    UC_InvenComponent* InvenComp = InPlayer->GetInvenComponent();
    if (!InvenComp) return;

    FInventoryEntry* Entry = InvenComp->GetSlotDataPtr(InItemIndex);
    if (!Entry) return;

    uint8 CurStatGrade = 0;
    // 유효성 및 최대 등급 체크
    if (!ValidateUpgradeTarget(Entry, TargetStat, CurStatGrade))
    {
        NotifyUpgradeFinished(InPlayer);
        return;
    }

    // 스탯 등급 증가 (실제 강화 실행)
    FUpgradableData* EquipmentData = Entry->GetEquipmentDataPtr();
    EquipmentData->AddStatGrade(TargetStat, 1);

    // 강화 재료 차감
    ConsumeUpgradeMaterials(InPlayer, InvenComp, Entry->ItemRowName, TargetStat, CurStatGrade);

    // 슬롯 갱신 마킹
	if (InPlayer->HasAuthority())
		InvenComp->MarkSlotDirty(InItemIndex);
	else
		InvenComp->OnInventorySlotChanged.Broadcast(InItemIndex, *Entry);
		
    // 장착 중인 무기면 동기화 처리
    UC_ItemManager* ItemManager = InPlayer->GetGameInstance()->GetSubsystem<UC_ItemManager>();
    if (ItemManager && InItemIndex < static_cast<int32>(EWeaponSlot::None))
    {
        UpdateEquippedWeaponData(InPlayer, ItemManager, Entry->ItemRowName, InItemIndex);
    }

    // 5. 완료 알림
    NotifyUpgradeFinished(InPlayer);
}

// ----------------------------------------------------------------------------
// Helper Functions
// ----------------------------------------------------------------------------

bool UC_ItemUpgradeComponent::ValidateUpgradeTarget(FInventoryEntry* InEntry, EUpgradableStats TargetStat, uint8& OutCurGrade)
{
    if (!InEntry || !InEntry->HasEquipmentData()) return false;

    FUpgradableData* EquipmentData = InEntry->GetEquipmentDataPtr();
    if (!EquipmentData) return false;

    OutCurGrade = EquipmentData->GetStatGrade(TargetStat);
    
    // 이미 최대 등급이면 강화 불가
    if (OutCurGrade >= MAX_GRADE) return false;

    return true;
}

void UC_ItemUpgradeComponent::ConsumeUpgradeMaterials(AC_BasicPlayer* InPlayer, UC_InvenComponent* InvenComp, FName ItemRowName, EUpgradableStats TargetStat, uint8 CurGrade)
{
    UC_ItemManager* ItemManager = InPlayer->GetGameInstance()->GetSubsystem<UC_ItemManager>();
    if (!ItemManager) return;

    const FItemUpgradeCostRow* UpgradeCostRow = ItemManager->GetWeaponUpgradeCostData(ItemRowName);
    if (!UpgradeCostRow) return;

    const FStatUpgradeCostInfo* CostInfo = UpgradeCostRow->GetTargetStatUpCostInfo(TargetStat);
    if (!CostInfo || !CostInfo->GradeCosts.IsValidIndex(CurGrade)) return;

    const FGradeCostInfo& CurrentRecipe = CostInfo->GradeCosts[CurGrade];

    for (const FUpgradeMaterialInfo& RequiredCost : CurrentRecipe.RequiredMaterials)
    {
        if (RequiredCost.MatterItemID.IsNone() || RequiredCost.RequiredCount <= 0) continue;
        
        InvenComp->Server_RemoveItemByRowName(RequiredCost.MatterItemID, RequiredCost.RequiredCount);
    }
}

void UC_ItemUpgradeComponent::UpdateEquippedWeaponData(AC_BasicPlayer* InPlayer, UC_ItemManager* ItemManager, FName ItemRowName, int32 InItemIndex)
{
    UC_EquippedComponent* EquipComp = InPlayer->GetEquippedComponent();
    if (!EquipComp) return;

    EWeaponSlot TargetSlot = static_cast<EWeaponSlot>(InItemIndex);
    AC_WeaponBase* CurWeapon = EquipComp->GetSlotWeapon(TargetSlot);
    const FWeaponData* WeaponData = ItemManager->GetWeaponData(ItemRowName);

    if (CurWeapon && WeaponData)
    {
        CurWeapon->InitializeItemData(WeaponData);

        if (InPlayer->IsLocallyControlled())
        {
            EquipComp->UpdateWeaponData(TargetSlot, ItemRowName);
        }
        else
        {
            EquipComp->Client_UpdateWeaponData(TargetSlot, ItemRowName);
        }
    }
}

void UC_ItemUpgradeComponent::NotifyUpgradeFinished(AC_BasicPlayer* InPlayer)
{
    AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(InPlayer->GetController());
    if (!PC) return;

    PC->SetIsUpgradingItem(false);

    if (PC->IsLocalPlayerController())
    {
        PC->FinishItemUpgrade();
    }
    else
    {
        PC->Client_FinishItemUpgrade();
    }
}

/*
void UC_ItemUpgradeComponent::UpgradeItem(AC_BasicPlayer* InPlayer, int32 InItemIndex, EUpgradableStats TargetStat)
{
	UC_InvenComponent* InvenComp = InPlayer->GetInvenComponent();

	FInventoryEntry* Entry = InvenComp->GetSlotDataPtr(InItemIndex);

	PRINT_LOCAL(GetWorld(), "UpgradeItem", FColor::Blue, 5.f);

	if (!Entry->HasEquipmentData()) return;

	//Entry.CustomData.GetPtr

	FEquipmentCustomData* EquipmentData = Entry->GetEquipmentDataPtr();

	if (!EquipmentData) return;
	
	const uint8 curStatGrade = EquipmentData->GetStatGrade(TargetStat);
	
	if (curStatGrade >= MAX_GRADE)
	{
		AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(InPlayer->GetController());
	
		PC->SetIsUpgrading(false);
		
		if (PC->IsLocalPlayerController())
			PC->FinishItemUpgrade();
		else
			PC->Client_FinishItemUpgrade();
		
		return;
	}	
	
	
	// <Upgrade!> 
	EquipmentData->AddStatGrade(TargetStat, 1);
	
	// TODO : 업그레이드 완료 되었으니 인벤에서 재료 소진해야 함.
	// 재료는 
	
	UC_ItemManager* ItemManager = InPlayer->GetGameInstance()->GetSubsystem<UC_ItemManager>();

	if (!ItemManager) return;
	
	// 어떤 아이템의 강화 재료 목록이 필요한 것 인지
	const FItemUpgradeCostRow* UpgradeCostRow = ItemManager->GetWeaponUpgradeCostData(Entry->ItemRowName);
	
	if (!UpgradeCostRow) return;
	
	// 어떤 스탯의 요구 재료 목록을 원하는지.
	const FStatUpgradeCostInfo* CostInfo = UpgradeCostRow->GetTargetStatUpCostInfo(TargetStat);
	
	if (!CostInfo) return;
	
	// 몇 단계를 가기 위한 요구 재료 목록을 원하는지.
	const FGradeCostInfo& CurrentRecipe = CostInfo->GradeCosts[curStatGrade];
	
	// 필요한 재료 목록을 돌면서 인벤에 소모 요청.
	for (const FUpgradeMaterialInfo& RequiredCost : CurrentRecipe.RequiredMaterials)
	{
		if (RequiredCost.MatterItemID.IsNone() || RequiredCost.RequiredCount <= 0) continue;
		
		// 인벤에게 해당 아이템 조정 요청.
		InvenComp->RemoveItemByRowName(RequiredCost.MatterItemID, RequiredCost.RequiredCount);
	}
	
	
	InvenComp->MarkSlotDirty(InItemIndex);

	// 장착 중인 아이템이면 AC_WeaponBase의 객체도 업데이트 해주어야 함.
	if (InItemIndex < static_cast<int32>(EWeaponSlot::None))
	{
		UC_EquippedComponent* EquipComp = InPlayer->GetEquippedComponent();

		AC_WeaponBase* CurWeapon = EquipComp->GetSlotWeapon(static_cast<EWeaponSlot>(InItemIndex));

		//UC_ItemManager* ItemManager = InPlayer->GetGameInstance()->GetSubsystem<UC_ItemManager>();

		//if (!ItemManager) return;

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
*/


