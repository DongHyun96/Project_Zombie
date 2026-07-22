#include "Actor/Components/ItemLinkComponent/C_ItemLinkComponent.h"

#include "GlobalData.h"
#include "Actor/Components/C_InvenComponent.h"
//#include "C_InvenComponent.h"

UC_ItemLinkComponent::UC_ItemLinkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UC_ItemLinkComponent::InitializeLink(UC_InvenComponent* InInvenComp, int32 InSlotIndex)
{
	OwningInvenComp = InInvenComp;
	SlotIndex = InSlotIndex;
}

void UC_ItemLinkComponent::ClearLink()
{
	OwningInvenComp = nullptr;
	SlotIndex = INDEX_NONE;
}

bool UC_ItemLinkComponent::IsLinkValid() const
{
	return OwningInvenComp != nullptr && SlotIndex != INDEX_NONE;
}

FInventoryEntry* UC_ItemLinkComponent::GetItemEntryPtr() const
{
	if (IsLinkValid())
	{
		return OwningInvenComp->GetSlotDataPtr(SlotIndex);
	}
	return nullptr;
}

FInventoryEntry UC_ItemLinkComponent::GetItemEntry() const
{
	if (FInventoryEntry* EntryPtr = GetItemEntryPtr())
	{
		return *EntryPtr;
	}
	return FInventoryEntry();
}