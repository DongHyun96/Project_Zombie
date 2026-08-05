// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Components/UpgradeComponent/C_StatUpgradeComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Actor/Components/C_PlayerStatComponent.h"
#include "Controller/C_BasicPlayerController.h"
#include "GameModeAndManager/C_ItemManager.h"


UC_StatUpgradeComponent::UC_StatUpgradeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UC_StatUpgradeComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UC_StatUpgradeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UC_StatUpgradeComponent::UpgradeItem(AC_BasicPlayer* InPlayer, const FName& UpStatName)
{
	//InPlayer->GetStatComponent()->
	if (!InPlayer || UpStatName.IsNone()) return;
	
	UC_PlayerStatComponent* PlayerStatComp = Cast<UC_PlayerStatComponent>(InPlayer->GetStatComponent());
	if (!PlayerStatComp) return;
	
	UC_ItemManager* ItemManager = InPlayer->GetGameInstance()->GetSubsystem<UC_ItemManager>();
	if (!ItemManager) return;
	
	const FPlayerStatUpgradeData* PSUData = ItemManager->GetPlayerStatUpgradeData(UpStatName);
	
	const uint8& curGrade = PlayerStatComp->GetStatGrade(UpStatName);
	
	// 스탯 상승 : TODO 아마 여기서 동기화까지 다 되고 있을 것
	PlayerStatComp->IncreaseStat(UpStatName, PSUData->GradeValue[curGrade]);
	
	// Stat Grade Up
	PlayerStatComp->IncreaseStatGrade(UpStatName);

	UC_InvenComponent* InvenComp = InPlayer->GetInvenComponent();
	
	if (!InvenComp) return;
	
	// 요구 재료 인벤에서 소모처리.
	const FGradeCostInfo& CurrentRecipe = PSUData->GradeCost[curGrade];

	for (const FUpgradeMaterialInfo& RequiredCost : CurrentRecipe.RequiredMaterials)
	{
		if (RequiredCost.MatterItemID.IsNone() || RequiredCost.RequiredCount <= 0) continue;
        
		InvenComp->Server_RemoveItemByRowName(RequiredCost.MatterItemID, RequiredCost.RequiredCount);
	}
	
	AC_BasicPlayerController* PC = Cast<AC_BasicPlayerController>(InPlayer->GetController());
	
	if (!PC) return;

	PC->SetIsUpgradingPlayerStat(false);

	if (PC->IsLocalPlayerController())
	{
		PC->FinishPlayerStatUpgrade();
	}
	else
	{
		PC->Client_FinishPlayerStatUpgrade();
	}
}

