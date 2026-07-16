// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeAndManager/C_ItemManager.h"
#include "../Item/PickUp/C_ItemPickUp.h"
void UC_ItemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 런타임(Initialize 함수)이므로 StaticLoadObject를 사용합니다.
    UDataTable* LoadedDataTable = Cast<UDataTable>(StaticLoadObject(
        UDataTable::StaticClass(),
        nullptr,
        TEXT("/Game/SangYeon/Item/DT_ItemData_Sang.DT_ItemData_Sang")
    ));

    // 데이터 테이블 로드 성공
    if (LoadedDataTable)
    {
        ItemDataTable = LoadedDataTable;
        UE_LOG(LogTemp, Log, TEXT("ItemDataTableSuccessfully Loaded!"));
    }
    // 데이터 테이블 로드 실패
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load ItemDataTable! Check your asset path."));
    }
}

const FItemData* UC_ItemManager::GetItemData(FName InRowName) const
{
    if (!ItemDataTable || InRowName.IsNone()) return nullptr;

    // FindRow는 내부적으로 TMap 해시 조회를 하기 때문에 매우 빠릅니다.
    return ItemDataTable->FindRow<FItemData>(InRowName, TEXT("GetItemDataContext"));
}

AC_ItemPickUp* UC_ItemManager::SpawnItem(FName InRowName, const FVector& SpawnLocation, int32 InCount, const FVector& LaunchVelocity)
{
    // 1. 안전성 검사 및 데이터 가져오기
    const FItemData* Data = GetItemData(InRowName);
    if (!Data) return nullptr;

    UWorld* World = GetWorld();
    if (!World) return nullptr;

    // 2. 월드에 기본 아이템 액터 스폰 (AItemActor는 월드에 떨어질 공통 베이스 액터)
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // TSubclassOf<AItemActor> 등으로 지정된 클래스를 스폰 (여기서는 가상의 스폰 예시)
    AC_ItemPickUp* NewItem = World->SpawnActor<AC_ItemPickUp>(AC_ItemPickUp::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);

    if (NewItem)
    {
        // 3. 생성된 아이템 액터에 데이터 테이블 정보를 주입!
        // (예: 아이템 에셋의 StaticMesh를 바꾸거나, 수량을 설정하는 함수 호출)
        NewItem->ItemData.ItemRowName = InRowName;
        NewItem->ItemData.CurCount = InCount;
        NewItem->SetMeshRef(Data->DropMesh);
        NewItem->SetPickupMeshAsync(NewItem->GetMeshRef());
        
        // --- 마인크래프트식 툭 던지기 물리 로직 추가 ---
        // AC_ItemPickUp 내부에 루트 컴포넌트 혹은 핵심 메쉬 컴포넌트(예: StaticMeshComponent)가 있다고 가정합니다.
        if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(NewItem->GetRootComponent()))
        {
            // 1. 물리를 켭니다. (에디터에서 미리 켜두어도 됩니다)
            RootPrim->SetSimulatePhysics(true);
            
            // 2. 만약 힘이 전달되었다면 해당 방향으로 툭 던집니다.
            if (!LaunchVelocity.IsNearlyZero())
            {
                // Velocity를 즉각 변화시키는 Impulse 방식을 사용합니다.
                // VelChange를 true로 하면 질량(Mass)에 상관없이 일정한 속도로 날아갑니다.
                RootPrim->AddImpulse(LaunchVelocity, NAME_None, true);
            }
        }
    }

    return NewItem;
}
