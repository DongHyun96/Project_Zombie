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

AC_ItemPickUp* UC_ItemManager::SpawnItemPickUp(FName InRowName, int32 InCount, const FVector& SpawnLocation)
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
    
    AC_ItemPickUp* NewItem = SpawnItemPickUp(InRowName, InCount, SpawnLocation);
    if (NewItem)
    {
        AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(InActor);
        if (Player)
        {
            Player->GetInvenComponent()->GetItemAt()[Slot]
            NewItem->ItemEntry = 
        }
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
        // TODO: Throwable 테이블 연결 시 작성
        break;

    case EItemType::CONSUMABLE:
    case EItemType::MATTER:
    case EItemType::GADGET:
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
        SpawnedWeapon->InitializeItemActor(InRawData);
    }

    return SpawnedWeapon;
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
