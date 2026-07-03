// Fill out your copyright notice in the Description page of Project Settings.


#include "Multi/C_InvenStructures.h"

#include "Actor/Components/C_InvenComponent.h"
#include "Utility/C_Util.h"

void FInventoryContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalDelta)
{
	
	for (int32 Index : AddedIndices)
	{
		if (OwnerComponent && Items.IsValidIndex(Index))
		{
			// UI 슬롯 위젯에 "이 슬롯 주소 정보 바뀌었어!" 라고 브로드캐스트
			OwnerComponent->OnInventorySlotChanged.Broadcast(Items[Index].SlotIndex, Items[Index]);
			UC_Util::Print("PostReplAdd");
		}
	}
}

void FInventoryContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalDelta)
{
	for (int32 Index : ChangedIndices)
	{
		if (OwnerComponent && Items.IsValidIndex(Index))
		{
			// 아이템이 바뀌거나 수량이 변했을 때 UI 슬롯 브로드캐스트
			OwnerComponent->OnInventorySlotChanged.Broadcast(Items[Index].SlotIndex, Items[Index]);
			UC_Util::Print("PostReplChange");
			
		}
	}
}