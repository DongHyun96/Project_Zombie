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

void UC_StatUpgradeComponent::UpgradeStat(AC_BasicPlayer* InPlayer, const FName& UpStatName)
{
    if (!InPlayer || UpStatName.IsNone()) return;

    UC_PlayerStatComponent* PlayerStatComp = Cast<UC_PlayerStatComponent>(InPlayer->GetStatComponent());
    if (!PlayerStatComp) return;

    UC_ItemManager* ItemManager = InPlayer->GetGameInstance()->GetSubsystem<UC_ItemManager>();
    if (!ItemManager) return;

    const FPlayerStatUpgradeData* PSUData = ItemManager->GetPlayerStatUpgradeData(UpStatName);
    if (!PSUData) return; // 데이터 테이블 Null 안전장치 추가

    const uint8 curGrade = PlayerStatComp->GetStatGrade(UpStatName);

    // 1. 등급 체크
    if (curGrade >= MAX_GRADE)
    {
        FinishUpgradeState(InPlayer);
        return;
    }

    // 2. 스탯 증가 및 등급 상승
    if (PSUData->GradeValue.IsValidIndex(curGrade))
    {
        PlayerStatComp->IncreaseStat(UpStatName, PSUData->GradeValue[curGrade]);
		PlayerStatComp->IncreaseStatGrade(UpStatName);
    }
	

    // 3. 재료 소모 처리
    if (PSUData->GradeCost.IsValidIndex(curGrade))
    {
        ConsumeUpgradeMaterials(InPlayer->GetInvenComponent(), PSUData->GradeCost[curGrade]);
    }

    // 4. 상태 종료
    FinishUpgradeState(InPlayer);
}

void UC_StatUpgradeComponent::ConsumeUpgradeMaterials(UC_InvenComponent* InvenComp, const FGradeCostInfo& CostInfo)
{
    if (!InvenComp) return;

    for (const FUpgradeMaterialInfo& RequiredCost : CostInfo.RequiredMaterials)
    {
        if (RequiredCost.MatterItemID.IsNone() || RequiredCost.RequiredCount <= 0) continue;

        InvenComp->Server_RemoveItemByRowName(RequiredCost.MatterItemID, RequiredCost.RequiredCount);
    }
}

void UC_StatUpgradeComponent::FinishUpgradeState(AC_BasicPlayer* InPlayer)
{
    if (!InPlayer) return;

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

/*
void UC_StatUpgradeComponent::UpgradeStat(AC_BasicPlayer* InPlayer, const FName& UpStatName)
{
	//InPlayer->GetStatComponent()->
	if (!InPlayer || UpStatName.IsNone()) return;
	
	UC_PlayerStatComponent* PlayerStatComp = Cast<UC_PlayerStatComponent>(InPlayer->GetStatComponent());
	if (!PlayerStatComp) return;
	
	UC_ItemManager* ItemManager = InPlayer->GetGameInstance()->GetSubsystem<UC_ItemManager>();
	if (!ItemManager) return;
	
	const FPlayerStatUpgradeData* PSUData = ItemManager->GetPlayerStatUpgradeData(UpStatName);
	
	const uint8& curGrade = PlayerStatComp->GetStatGrade(UpStatName);
	
	if (curGrade >= MAX_GRADE)
	{
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
		return;
	}
	
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
}*/
