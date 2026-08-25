// Fill out your copyright notice in the Description page of Project Settings.


#include "Multi/C_InvenStructures.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Utility/C_Util.h"

void FInventoryContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalDelta)
{
	for (int32 Index : AddedIndices)
	{
		if (OwnerComponent && Items.IsValidIndex(Index))
		{
			OwnerComponent->OnInventorySlotChanged.Broadcast(Items[Index].SlotIndex, Items[Index]);
		}
	}
}

void FInventoryContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalDelta)
{
	for (int32 Index : ChangedIndices)
	{
		if (OwnerComponent && Items.IsValidIndex(Index))
		{
			OwnerComponent->OnInventorySlotChanged.Broadcast(Items[Index].SlotIndex, Items[Index]);
		}
	}
}
