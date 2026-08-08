// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GameMode_GameLv.h"

#include "C_ZombieManager.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Actor/Components/StatComponent/C_StatComponentBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerState/C_PlayerState.h"
#include "PointTowerManager/C_PointTowerManager.h"
#include "Utility/C_Util.h"

AC_GameMode_GameLv::AC_GameMode_GameLv()
{
   PlayerStateClass = AC_PlayerState::StaticClass();
   bUseSeamlessTravel = true;
}

void AC_GameMode_GameLv::BeginPlay()
{
    Super::BeginPlay();

    if (m_ZombieManagerClass)
       m_ZombieManager = NewObject<UC_ZombieManager>(this, m_ZombieManagerClass);
    else
       m_ZombieManager = NewObject<UC_ZombieManager>(this);

    if (m_ZombieManager) m_ZombieManager->OnWorldBeginPlay();
    
    m_PointTowerManager = NewObject<UC_PointTowerManager>(this);
    m_PointTowerManager->OnWorldBeginPlay();
}   

void AC_GameMode_GameLv::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    
    AC_PlayerState* PState = Cast<AC_PlayerState>(NewPlayer->PlayerState);
    if (!PState) return;

    PState->m_bIsHost = NewPlayer->IsLocalController();
}

void AC_GameMode_GameLv::Logout(AController* Exiting)
{
    APlayerController* PC = Cast<APlayerController>(Exiting);
    if (PC)
    {
       AC_PlayerState* PS = PC->GetPlayerState<AC_PlayerState>();
       if (PS)
       {
          const int32 LeaverPlayerID = PS->GetPlayerId();
          UWorld* CurrentWorld = GetWorld();

          for (TObjectIterator<UC_InvenComponent> It; It; ++It)
          {
             UC_InvenComponent* InvenComp = *It;
             if (InvenComp && !InvenComp->IsTemplate() && InvenComp->GetWorld() == CurrentWorld)
             {
                InvenComp->ReleaseAllLocksByPlayer(LeaverPlayerID);
             }
          }
       }
    }
    Super::Logout(Exiting);
}

void AC_GameMode_GameLv::HandleSeamlessTravelPlayer(AController*& C)
{
   // 1. 엔진 로직 선행 (캐릭터 스폰 및 빙의 완료)
   Super::HandleSeamlessTravelPlayer(C);

   APlayerController* PC = Cast<APlayerController>(C);
   if (!PC) return;

   AC_PlayerState* PS = PC->GetPlayerState<AC_PlayerState>();
   if (!PS) return;

   APawn* NewPawn = PC->GetPawn();
   if (!NewPawn) return;

   // 2. 캐릭터의 PossessedBy에서 복구하지 못했을 경우를 대비해 확실하게 다시 강제 로드
   // (심리스 트래블 완료 시점의 확실한 데이터 복구 보장)
   if (UC_InvenComponent* InvenComp = NewPawn->FindComponentByClass<UC_InvenComponent>())
   {
      // 기존 인벤토리가 비어있거나, 데이터 복구가 누락되었다면 여기서 꽂아줌
      InvenComp->LoadInventoryFromBackup(PS->GetSavedInventory()); 
      UE_LOG(LogTemp, Warning, TEXT("[Travel Restore] 게임모드 단계에서 새 캐릭터 %s로 인벤토리 최종 복구 완료! (아이템: %d개)"), 
         *NewPawn->GetName(), PS->GetSavedInventory().Num());
   }

   if (UC_StatComponentBase* StatComp = NewPawn->FindComponentByClass<UC_StatComponentBase>())
   {
      StatComp->LoadStatsFromBackup(PS->GetSavedStats(), PS->GetSavedStatGrades());
      UE_LOG(LogTemp, Warning, TEXT("[Travel Restore] 게임모드 단계에서 새 캐릭터 %s로 스탯 최종 복구 완료!"), *NewPawn->GetName());
   }
}
