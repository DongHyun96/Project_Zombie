// C_ItemManager.cpp
#include "GameModeAndManager/C_ItemManager.h"
#include "../Item/PickUp/C_ItemPickUp.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Actor/Components/ItemLinkComponent/C_ItemLinkComponent.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "ProjectSettings/C_ItemManagerSettings.h"
#include "Utility/C_Util.h"

void UC_ItemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const UC_ItemManagerSettings* Settings = GetDefault<UC_ItemManagerSettings>();
    if (!Settings) return;

    // Helper Lambda: SoftObjectPtr 동기 로드 후 Map 등록
    auto RegisterTable = [this](EItemTableType Type, const TSoftObjectPtr<UDataTable>& SoftTablePtr)
    {
        if (SoftTablePtr.IsNull()) return;

        if (UDataTable* LoadedTable = SoftTablePtr.LoadSynchronous())
        {
            CachedItemTables.Add(Type, LoadedTable);
        }
    };

    // 설정된 데이터 테이블 동기 로드 및 캐싱
    RegisterTable(EItemTableType::General, Settings->GeneralItemDataTableConfig);
    RegisterTable(EItemTableType::Gun, Settings->GunDataTableConfig);
    RegisterTable(EItemTableType::Melee, Settings->MeleeDataTableConfig);
    RegisterTable(EItemTableType::Throwable, Settings->ThrowableDataTableConfig);
    RegisterTable(EItemTableType::Potion, Settings->PotionDataTableConfig);

    // 강화 데이터 테이블 캐싱 (ItemManager가 계속 관리)
    if (!Settings->WeaponUpgradePerValueTableConfig.IsNull())
    {
        WeaponUpgradeData = Settings->WeaponUpgradePerValueTableConfig.LoadSynchronous();
    }
    
    // 아이템 강화 재료 데이터 테이블 캐싱
    if (!Settings->WeaponUpgradeCostTableConfig.IsNull())
    {
        ItemUpgradeCostData = Settings->WeaponUpgradeCostTableConfig.LoadSynchronous();
    }
    
    // PlayerStatUpgradeData 데이터 테이블 캐싱
    if (!Settings->FPlayerStatUpgradeDataTableConfig.IsNull())
    {
        PlayerStatUpgradeData = Settings->FPlayerStatUpgradeDataTableConfig.LoadSynchronous();
    }
}

void UC_ItemManager::ReturnToPool(AC_ItemPickUp* ItemToReturn)
{
    if (!IsValid(ItemToReturn)) return;

    // 활성화 목록에서 제거
    ActiveItemPool.Remove(ItemToReturn);

    // 비활성화 처리 (시각/물리/타이머 끄기)
    ItemToReturn->DeactivateItem();

    // 풀 배열에 보관
    InactiveItemPool.AddUnique(ItemToReturn);
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

AC_ItemPickUp* UC_ItemManager::SpawnItemPickUp(FName InRowName, int32 InCount, const FVector& SpawnLocation)
{
    FInventoryEntry TempEntry;
    TempEntry.ItemRowName = InRowName;
    TempEntry.CurCount = InCount;

    // Entry 버전을 호출해서 코드 중복 제거!
    return SpawnItemPickUp(TempEntry, SpawnLocation);
}

AC_ItemPickUp* UC_ItemManager::SpawnItemPickUp(const FInventoryEntry& InEntry, const FVector& SpawnLocation)
{
    //if (InEntry.ItemRowName.IsNone() || InEntry.CurCount <= 0) return nullptr;
//
    //const FItemData* Data = GetItemData<FItemData>(EItemTableType::General, InEntry.ItemRowName);
    //if (!Data) return nullptr;
//
    //UWorld* World = GetWorld();
    //if (!World) return nullptr;
//
    //FActorSpawnParameters SpawnParams;
    //SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    //
    //AC_ItemPickUp* NewItem = World->SpawnActor<AC_ItemPickUp>(AC_ItemPickUp::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    //if (NewItem)
    //{
    //    // 1. InEntry 데이터 복사 (여기서 CustomData 포인터/값 및 동적 정보가 통째로 전달됨)
    //    NewItem->ItemEntry = InEntry; 
//
    //    // 2. [핵심] 만약 인벤토리 Entry의 CustomData가 비어있는 상태로 새로 스폰된 아이템이라면?
    //    // -> 데이터 테이블(FItemData)에 지정된 기본 CustomData(FEquipmentCustomData)를 넣어준다!
    //    if (!NewItem->ItemEntry.CustomData.IsValid())
    //    {
    //        NewItem->ItemEntry.CustomData = FInstancedStruct::Make(Data->CustomData);
    //    }
//
    //    NewItem->SetMeshRef(Data->DropMesh);
    //    NewItem->SetPickupMeshAsync(NewItem->GetMeshRef());
    //}
    //
    //return NewItem;
    
    return GetOrCreateItemPickUp(InEntry, SpawnLocation);
}

AC_ItemPickUp* UC_ItemManager::GetOrCreateItemPickUp(const FInventoryEntry& InEntry, const FVector& SpawnLocation)
{
    if (InEntry.ItemRowName.IsNone() || InEntry.CurCount <= 0) return nullptr;

    const FItemData* Data = GetItemData<FItemData>(EItemTableType::General, InEntry.ItemRowName);
    if (!Data) return nullptr;

    UWorld* World = GetWorld();
    if (!World) return nullptr;

    // [미래 가능성 열어두기] 만약 활성화 수량이 한계치에 다다르면?
    // (현재는 적용 안 함, 추후 필요 시 주석 해제하여 사용)
    /*
    if (ActiveItemPool.Num() >= MaxActiveItemLimit)
    {
        // 가장 오래된 아이템 하나를 강제 수거하는 로직 등
    }
    */

    AC_ItemPickUp* TargetItem = nullptr;

    // 1. 풀에 재사용 가능한 액터가 있는지 확인
    while (InactiveItemPool.Num() > 0)
    {
        AC_ItemPickUp* PooledCandidate = InactiveItemPool.Pop(false);
        if (IsValid(PooledCandidate))
        {
            TargetItem = PooledCandidate;
            break;
        }
    }

    // 2. 풀에 없으면 새로 스폰 (최초 스폰)
    if (!TargetItem)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        TargetItem = World->SpawnActor<AC_ItemPickUp>(AC_ItemPickUp::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    }

    if (TargetItem)
    {
        ActiveItemPool.Add(TargetItem);

        // CustomData 세팅
        FInventoryEntry FinalEntry = InEntry;
        if (!FinalEntry.CustomData.IsValid())
        {
            FinalEntry.CustomData = FInstancedStruct::Make(Data->CustomData);
        }

        // 3. 풀링 전용 활성화 호출
        TargetItem->ActivateItem(FinalEntry, SpawnLocation);

        // 4. 비동기 메시 로드 및 적용 (기존 로직 그대로 유지)
        TargetItem->SetMeshRef(Data->DropMesh);
        TargetItem->SetPickupMeshAsync(TargetItem->GetMeshRef());
    }

    return TargetItem;
}

bool UC_ItemManager::DropItemByPlayer(const FInventoryEntry& InEntry, AActor* InActor)
{
    if (!InActor || InEntry.ItemRowName.IsNone() || InEntry.CurCount <= 0) return false;
    
    FVector SpawnLocation = InActor->GetActorLocation() + FVector(0.f, 0.f, 30.f);
    FVector ForwardVec = InActor->GetActorForwardVector();
    FVector UpVec = InActor->GetActorUpVector();
    
    SpawnLocation += ForwardVec * 100.f;
    
    // Core 스폰 함수 호출
    AC_ItemPickUp* NewItem = SpawnItemPickUp(InEntry, SpawnLocation);
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
    
    UC_Util::Print(InactiveItemPool.Num());
    
    return NewItem != nullptr;
}

bool UC_ItemManager::DropItemByPlayer(FName InRowName, int32 InCount, AActor* InActor)
{
    FInventoryEntry TempEntry;
    TempEntry.ItemRowName = InRowName;
    TempEntry.CurCount = InCount;

    // Entry 기반 드롭으로 전달
    return DropItemByPlayer(TempEntry, InActor);
}

AC_WeaponBase* UC_ItemManager::SpawnEquippedActor(FName InRowName, AActor* InOwner, const FTransform& SpawnTransform)
{
    if (InRowName.IsNone()) return nullptr;

    UWorld* World = GetWorld();
    if (!World) return nullptr;

    // 1차: General 테이블(FItemData) 검증
    const FItemData* GeneralData = GetItemData<FItemData>(EItemTableType::General, InRowName);
    if (!GeneralData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ItemManager] '%s' Row is missing in General Table!"), *InRowName.ToString());
        return nullptr;
    }
    
    AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(InOwner);
        
    if (!Player) return nullptr;
        
    UC_InvenComponent* InvenComp = Cast<UC_InvenComponent>(Player->GetInvenComponent());
        
    if (!InvenComp) return nullptr;
    
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = InOwner;
    SpawnParams.Instigator = Cast<APawn>(InOwner);
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    TSubclassOf<AC_WeaponBase> TargetClass = nullptr;

    // 2차: EItemType에 따른 세부 테이블 및 Class 추출
    
    int32 SlotIdx = -1;
    
    const FWeaponData* InRawData = nullptr;
    switch (GeneralData->ItemType)
    {
    case EItemType::MAINWEAPON:
        if (const FGunData* GunData = GetItemData<FGunData>(EItemTableType::Gun, InRowName))
        {
            TargetClass = GunData->EquippedActorClass;
            SlotIdx = 0;
            InRawData = GunData;
        }
        break;

    case EItemType::MELEEWEAPON:
        if (const FMeleeData* MeleeData = GetItemData<FMeleeData>(EItemTableType::Melee, InRowName))
        {
            TargetClass = MeleeData->EquippedActorClass;
            SlotIdx = 1;
            InRawData = MeleeData;
        }
        break;

    case EItemType::THROWABLE:
        if (const FThrowableData* ThrowableData = GetItemData<FThrowableData>(EItemTableType::Throwable, InRowName))
        {
            TargetClass = ThrowableData->EquippedActorClass;
            SlotIdx = 2;
            InRawData = ThrowableData;
        }
        break;
        
    case EItemType::POTION:
        if (const FPotionData* PotionData = GetItemData<FPotionData>(EItemTableType::Potion, InRowName))
        {
            TargetClass = PotionData->EquippedActorClass;
            SlotIdx = 3;
            InRawData = PotionData;
        }
        break;
    case EItemType::CONSUMABLE:
    case EItemType::MATTER:
    default:
        break;
    }

    if (!TargetClass || SlotIdx == -1)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ItemManager] Failed to resolve EquippedActorClass for Row: %s"), *InRowName.ToString());
        return nullptr;
    }
    
    // 무기 스폰 및 ItemEntry 주입 초기화
    AC_WeaponBase* SpawnedWeapon = World->SpawnActor<AC_WeaponBase>(TargetClass, SpawnTransform, SpawnParams);
    if (SpawnedWeapon)
    {
        // 스폰 된 무기의 ItemLinkComponent의 초기화
        UC_ItemLinkComponent* LinkComp = SpawnedWeapon->GetLinkComp();
        
        if (!LinkComp)
        {
            UC_Util::Print("ItemLinkComponent of SpawnedWeapon is nullptr in SpawnedWeapon Spawn Equipped Actor", FColor::Red, 10.f);
            SpawnedWeapon->Destroy();
            return nullptr;
        }
        
        LinkComp->InitializeLink(InvenComp, SlotIdx);
        
        // 무기의 초기화
        if (Player->IsLocallyControlled())
            SpawnedWeapon->SetItemRowName(InRowName);    
        
        SpawnedWeapon->InitializeItemActor(InRawData);
        
        if (!Player->IsLocallyControlled())
            SpawnedWeapon->SetItemRowName(InRowName);
    }

    return SpawnedWeapon;
}

const FWeaponData* UC_ItemManager::GetWeaponData(FName InRowName) const
{
    // 1차: General 테이블에서 ItemType 확인
    const FItemData* GeneralData = GetItemData<FItemData>(EItemTableType::General, InRowName);

    if (!GeneralData) return nullptr;

    // 2차: ItemType에 맞춰 알맞은 테이블에서 FWeaponData 가져오기
    const FWeaponData* WeaponData = nullptr;
    switch (GeneralData->ItemType)
    {
    case EItemType::MAINWEAPON:
        WeaponData = GetItemData<FGunData>(EItemTableType::Gun, InRowName);
        break;
    case EItemType::MELEEWEAPON:
        WeaponData = GetItemData<FMeleeData>(EItemTableType::Melee, InRowName);
        break;
    case EItemType::THROWABLE:
        WeaponData = GetItemData<FThrowableData>(EItemTableType::Throwable, InRowName);
        break;
    case EItemType::POTION:
        WeaponData = GetItemData<FPotionData>(EItemTableType::Potion, InRowName);
        break;
    default:
        break;
    }
    return WeaponData;
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
        if (const FThrowableData* Ptr = GetItemData<FThrowableData>(InTableType, InRowName))
        {
            OutData.InitializeAs<FThrowableData>(*Ptr);
            return true;
        }
        break;
    case EItemTableType::Potion:
        if (const FPotionData* Ptr = GetItemData<FPotionData>(InTableType, InRowName))
        {
            OutData.InitializeAs<FPotionData>(*Ptr);
        }
    }
    return false;
}
