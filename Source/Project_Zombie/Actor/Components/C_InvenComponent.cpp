#include "Actor/Components/C_InvenComponent.h"
//#include "Net/UnrealNetwork.h"

UC_InvenComponent::UC_InvenComponent()
{

	PrimaryComponentTick.bCanEverTick = false;


}


void UC_InvenComponent::BeginPlay()
{
	Super::BeginPlay();

	// ◀ 컴포넌트 자체가 네트워크 복제가 되도록 설정해야 내부 변수도 복제됩니다.
	//SetIsReplicatedByDefault(true);
}


void UC_InvenComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

bool UC_InvenComponent::AddItem(FInventoryEntry ItemEntry)
{
	if (InventoryItems.Num() > 180) return false;

	// TODO : 같은 아이템이 인벤토리에 존재한다면 거기에 추가해주기.

	for (int i = 0; i < 180; ++i)
	{
		// TODO : 겹칠 수 없는 아이템은 예외처리하기.
		if (InventoryItems[i].ItemRowName == ItemEntry.ItemRowName)
		{
			if (InventoryItems[i].bIsStack == true)
			{
				InventoryItems[i].Count += ItemEntry.Count;
				return true;
			}
		}
	}

	InventoryItems.Add(ItemEntry);

	return true;
}

//void UC_InvenComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	// InventoryItems 배열이 서버에서 클라이언트로 복제되도록 등록
//	DOREPLIFETIME(UC_InvenComponent, InventoryItems);
//}

