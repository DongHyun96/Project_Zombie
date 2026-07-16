// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/C_BasicPlayerController.h"

#include "Actor/Components/C_InvenComponent.h"
#include "GameFramework/PlayerState.h"


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
