// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/C_BasicPlayerController.h"

#include "Actor/Components/C_InvenComponent.h"
#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "GameFramework/PlayerState.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/PlayerState/C_PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "UI/InvenUI/Upgrade/C_ItemUpgradeWidget.h"
#include "UI/InvenUI/Upgrade/C_PlayerStatUpgradeWidget.h"
#include "UI/MainHUD/C_GameMainHUD.h"


void AC_BasicPlayerController::OnUnPossess()
{
	
	// 심리스 트래블 시 컨트롤러가 기존 폰과 분리되기 직전에 호출됩니다.
	if (APawn* prevPawn = GetPawn())
	{
		if (AC_PlayerState* PS = GetPlayerState<AC_PlayerState>())
		{
			// 1. 인벤토리 백업
			if (UC_InvenComponent* InvenComp = prevPawn->FindComponentByClass<UC_InvenComponent>())
			{
				PS->SaveInventoryToState(InvenComp->GetInventoryItems());
				UE_LOG(LogTemp, Warning, TEXT("[Travel Save] 옛날 캐릭터 %s의 인벤토리 백업 완료 (아이템: %d개)"), 
					*prevPawn->GetName(), InvenComp->GetInventoryItems().Num());
			}

			// 2. 스탯 백업
			if (UC_StatComponentBase* StatComp = prevPawn->FindComponentByClass<UC_StatComponentBase>())
			{
				PS->SaveStatsToState(StatComp->GetStatsMap(), StatComp->GetStatGradesMap());
				UE_LOG(LogTemp, Warning, TEXT("[Travel Save] 옛날 캐릭터 %s의 스탯 데이터 백업 완료"), *prevPawn->GetName());
			}
		}
	}

	Super::OnUnPossess();
}

void AC_BasicPlayerController::FinishItemUpgrade()
{
	AC_UIManager* UIManager = AC_UIManager::Get(GetWorld());
	
	if (!UIManager) return;
	
	UC_InventoryWidget* InvenWidget = UIManager->GetInventoryWidget();
	
	if (!InvenWidget) return;
	
	UC_ItemUpgradeWidget* ItemUpgradeWidget = InvenWidget->GetItemUpgradeWidget();
	
	if (!ItemUpgradeWidget) return;
	
	ItemUpgradeWidget->SetIsUpgrading(false);
	
	//UIManager->GetMainHUDWidget()->UpdateLeftAmmoTotalCount()
	
	//ItemUpgradeWidget->UpdateWidget();
	
	PRINT_LOCAL(GetWorld(), "FinishItemUpgrade", FColor::Red, 5.f);
}

void AC_BasicPlayerController::FinishPlayerStatUpgrade()
{
	AC_UIManager* UIManager = AC_UIManager::Get(GetWorld());
	
	if (!UIManager) return;
	
	UC_InventoryWidget* InvenWidget = UIManager->GetInventoryWidget();
	
	if (!InvenWidget) return;
	
	UC_PlayerStatUpgradeWidget* PlayerStatUpgradeWidget = InvenWidget->GetPlayerStatUpgradeWidget();
	
	if (!PlayerStatUpgradeWidget) return;
	
	PlayerStatUpgradeWidget->SetIsUpgrading(false);
	
	//PlayerStatUpgradeWidget->UpdateWidget();
	
	//UIManager->GetMainHUDWidget()->UpdateLeftAmmoTotalCount()
	
	//ItemUpgradeWidget->UpdateWidget();
	
	PRINT_LOCAL(GetWorld(), "FinishItemUpgrade", FColor::Red, 5.f);
}

void AC_BasicPlayerController::OnRep_IsUpgradingPlayerStat()
{
	FinishPlayerStatUpgrade();
}

void AC_BasicPlayerController::Client_FinishPlayerStatUpgrade_Implementation()
{
	FinishPlayerStatUpgrade();
}

void AC_BasicPlayerController::OnRep_IsUpgradingItem()
{
	FinishItemUpgrade();
}

void AC_BasicPlayerController::Client_FinishItemUpgrade_Implementation()
{
	FinishItemUpgrade();
}

void AC_BasicPlayerController::Destroyed()
{
	// 중요: 부모 Destroyed가 호출되기 전에 실행
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] PC Destroyed! Checking active dragged item..."));

		// 내가 잠가둔 인벤토리 정보가 남아있다면 즉시 해제
		if (Server_ActiveDraggedInven && Server_ActiveDraggedSlotIndex != -1)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Server] PC Force Releasing Slot %d in Inven %s"), 
				Server_ActiveDraggedSlotIndex, *Server_ActiveDraggedInven->GetName());

			// 인벤토리 구조체 내부의 LockedByPlayerID를 INDEX_NONE으로 밀어버리고 리플리케이션 트리거
			// (강제 종료 상황이므로 슬롯 검증 없이 무조건 락을 풉니다)
			Server_ActiveDraggedInven->ForceReleaseSlotLock(Server_ActiveDraggedSlotIndex);
		}
	}

	Super::Destroyed();
}

void AC_BasicPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AC_BasicPlayerController, bIsUpgradingItem);
}
