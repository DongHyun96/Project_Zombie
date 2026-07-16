// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GameMode_GameLv.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utility/C_Util.h"

void AC_GameMode_GameLv::Logout(AController* Exiting)
{
	// [중요] 부모 로그아웃(Super::Logout)을 호출하기 전에 우리의 로직을 먼저 실행해야 합니다!
	APlayerController* PC = Cast<APlayerController>(Exiting);
	if (PC)
	{
		APlayerState* PS = PC->GetPlayerState<APlayerState>();
		if (PS)
		{
			int32 LeaverPlayerID = PS->GetPlayerId();
            
			// 디버그 로그: 정말 이 단계까지 진입하는지 확인
			UE_LOG(LogTemp, Warning, TEXT("Logout Detected! Leaver Player ID: %d"), LeaverPlayerID);
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Logout Detected! PlayerID: %d"), LeaverPlayerID), true, true, FLinearColor::Red, 10.f);

			UWorld* CurrentWorld = GetWorld();

			// 월드 안의 인벤토리 컴포넌트들을 돌며 락 해제
			for (TObjectIterator<UC_InvenComponent> It; It; ++It)
			{
				UC_InvenComponent* InvenComp = *It;
				if (InvenComp && !InvenComp->IsTemplate() && InvenComp->GetWorld() == CurrentWorld)
				{
					InvenComp->ReleaseAllLocksByPlayer(LeaverPlayerID);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Logout: PlayerState is Null! Cannot get PlayerID."));
		}
	}

	// 모든 작업이 끝난 뒤 부모 로그아웃 호출
	Super::Logout(Exiting);
}
