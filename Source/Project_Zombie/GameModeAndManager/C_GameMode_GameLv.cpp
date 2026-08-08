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
   // 1. 엔진 로직 선행 (목적지 레벨에 새 캐릭터 스폰 및 빙의 완료)
   Super::HandleSeamlessTravelPlayer(C);

   APlayerController* PC = Cast<APlayerController>(C);
   if (!PC) return;

   // CopyProperties()를 통해 이미 이전 레벨의 데이터를 고스란히 이어받은 상태의 새 PlayerState
   AC_PlayerState* PS = PC->GetPlayerState<AC_PlayerState>();
   if (!PS) return;

   // 방금 막 스폰되어 컨트롤러가 갓 빙의한 새 깡통 캐릭터
   APawn* NewPawn = PC->GetPawn();
   if (!NewPawn) return;

   // 2. 새 캐릭터의 인벤토리 컴포넌트에 PlayerState가 품고 온 진짜 데이터 복구
   if (UC_InvenComponent* InvenComp = NewPawn->FindComponentByClass<UC_InvenComponent>())
   {
      // ※ 주의: 복구 함수(예: RestoreInventory)는 유저님이 구현하신 컴포넌트 내부의 데이터 세팅 함수명을 사용하세요.
      // 여기서는 예시로 로직을 적어둡니다. (PS->GetSavedInventoryItems() 등으로 가져오기)
      InvenComp->LoadInventoryFromBackup(PS->GetSavedInventory()); 
        
      UE_LOG(LogTemp, Warning, TEXT("[Travel Restore] 새 캐릭터 %s로 인벤토리 복구 완료!"), *NewPawn->GetName());
   }

   // 3. 새 캐릭터의 스탯 컴포넌트에 복구
   if (UC_StatComponentBase* StatComp = NewPawn->FindComponentByClass<UC_StatComponentBase>())
   {
      // 컴포넌트에 데이터를 다시 꽂아주는 함수 호출
      StatComp->LoadStatsFromBackup(PS->GetSavedStats(), PS->GetSavedStatGrades());
      UE_LOG(LogTemp, Warning, TEXT("[Travel Restore] 새 캐릭터 %s로 스탯 데이터 복구 완료!"), *NewPawn->GetName());
   }
}
