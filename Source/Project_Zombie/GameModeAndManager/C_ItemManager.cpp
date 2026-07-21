// C_ItemManager.cpp
#include "GameModeAndManager/C_ItemManager.h"
#include "../Item/PickUp/C_ItemPickUp.h"
#include "ProjectSettings/C_ItemManagerSettings.h"
#include "Utility/C_Util.h"

void UC_ItemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Settings 클래스로부터 설정 에셋 포인터 가져오기
    const UC_ItemManagerSettings* Settings = GetDefault<UC_ItemManagerSettings>();
    if (!Settings) return;

    // Helper Lambda: SoftObjectPtr을 로드하고 Map에 등록
    auto RegisterTable = [this](EItemTableType Type, const TSoftObjectPtr<UDataTable>& SoftTablePtr)
    {
        if (SoftTablePtr.IsNull()) return;

        UDataTable* LoadedTable = SoftTablePtr.LoadSynchronous();
        if (LoadedTable)
        {
            CachedItemTables.Add(Type, LoadedTable);
        }
    };

    // 설정된 데이터 테이블들을 동기 로드 및 캐싱
    RegisterTable(EItemTableType::General, Settings->GeneralItemDataTableConfig);
    RegisterTable(EItemTableType::Gun, Settings->GunDataTableConfig);
    RegisterTable(EItemTableType::Melee, Settings->MeleeDataTableConfig);
    RegisterTable(EItemTableType::Throwable, Settings->ThrowableDataTableConfig);
}

const UDataTable* UC_ItemManager::GetTargetTable(EItemTableType InTableType) const
{
    // Find는 const TMap에서 const TObjectPtr<UDataTable>* 를 반환합니다.
    if (const TObjectPtr<UDataTable>* FoundPtr = CachedItemTables.Find(InTableType))
    {
        return *FoundPtr; // TObjectPtr -> UDataTable* 자동 형변환
    }
    
    return nullptr;
}

AC_ItemPickUp* UC_ItemManager::SpawnItem(FName InRowName, int32 InCount, const FVector& SpawnLocation)
{
    const FItemData* Data = GetItemData<FItemData>(EItemTableType::General, InRowName);
    if (!Data) return nullptr;

    UWorld* World = GetWorld();
    if (!World) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AC_ItemPickUp* NewItem = World->SpawnActor<AC_ItemPickUp>(AC_ItemPickUp::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (NewItem)
    {
        NewItem->ItemEntry.ItemRowName = InRowName;
        NewItem->ItemEntry.CurCount = InCount;
        NewItem->SetMeshRef(Data->DropMesh);
        NewItem->SetPickupMeshAsync(NewItem->GetMeshRef());
    }
    
    return NewItem;
}

bool UC_ItemManager::DropItemByPlayer(FName InRowName, int32 InCount, AActor* InActor)
{
    if (!InActor) return false;
    
    FVector SpawnLocation = InActor->GetActorLocation() + FVector(0.f, 0.f, 30.f);
    FVector ForwardVec = InActor->GetActorForwardVector();
    FVector UpVec = InActor->GetActorUpVector();
    
    SpawnLocation += ForwardVec * 100.f;
    
    AC_ItemPickUp* NewItem = SpawnItem(InRowName, InCount, SpawnLocation);
    if (NewItem)
    {
        FVector LaunchVelocity = (ForwardVec * 300.f) + (UpVec * 150.f);
        LaunchVelocity.X += FMath::FRandRange(-50.f, 50.f);
        LaunchVelocity.Y += FMath::FRandRange(-50.f, 50.f);
        
        if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(NewItem->GetRootComponent()))
        {
            RootPrim->SetSimulatePhysics(true);
            if (!LaunchVelocity.IsNearlyZero())
            {
                RootPrim->AddImpulse(LaunchVelocity, NAME_None, true);
            }
        }
    }
    
    return NewItem != nullptr;
}

bool UC_ItemManager::GetItemDataBP(EItemTableType InTableType, FName InRowName, FInstancedStruct& OutData)
{
    switch (InTableType)
    {
    case EItemTableType::General:
        if (const FItemData* Ptr = GetItemData<FItemData>(InTableType, InRowName))
        {
            OutData.InitializeAs<FItemData>(*Ptr);
            return true;
        }
        break;

    case EItemTableType::Gun:
        if (const FGunData* Ptr = GetItemData<FGunData>(InTableType, InRowName))
        {
            OutData.InitializeAs<FGunData>(*Ptr);
            return true;
        }
        break;

    case EItemTableType::Melee:
        if (const FMeleeData* Ptr = GetItemData<FMeleeData>(InTableType, InRowName))
        {
            OutData.InitializeAs<FMeleeData>(*Ptr);
            return true;
        }
        break;

    case EItemTableType::Throwable:
        // TODO: FThrowableData 추가 시 작성
        break;
    }
    return false;
}