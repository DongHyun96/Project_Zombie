#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h" 
//#include "Net/Serialization/FastArraySerializer.h"
#include "C_InvenStructures.generated.h"

class UC_InvenComponent;

USTRUCT(BlueprintType)
struct FInventoryContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	// GlobalData.h에 정의된 FInventoryEntry 배열을 관리합니다.
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FInventoryEntry> Items;

	// 가방 컴포넌트를 가리키는 포인터
	UPROPERTY(Transient)
	UC_InvenComponent* OwnerComponent = nullptr;

	// 필수 직렬화 함수
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryEntry, FInventoryContainer>(Items, DeltaParms, *this);
	}

	// 클라이언트 동기화 콜백 함수들 (선언만!)
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalDelta) {}
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalDelta);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalDelta);
};

// 필수 매크로 등록
template<>
struct TStructOpsTypeTraits<FInventoryContainer> : public TStructOpsTypeTraitsBase2<FInventoryContainer>
{
	enum { WithNetDeltaSerializer = true };
};