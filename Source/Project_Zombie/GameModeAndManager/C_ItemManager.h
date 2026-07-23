#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GlobalData.h"
#include "C_ItemManager.generated.h"

class AC_WeaponBase;
class AC_ItemPickUp;
class UDataTable;

UCLASS()
class PROJECT_ZOMBIE_API UC_ItemManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
    
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	// --- SpawnItemPickUp 오버로딩 ---  바닥에 주울 수 있는 아이템으로 생성.
	// [1] Low-Level: 단순 템플릿 드롭, 상자, 몬스터 드롭용 (Name + Count)
	UFUNCTION(BlueprintCallable, Category = "ItemManager")
	AC_ItemPickUp* SpawnItemPickUp(FName InRowName, int32 InCount, const FVector& SpawnLocation);

	// [2] High-Level: 인벤토리 고유 데이터(내구도 등) 보존 스폰용 (FInventoryEntry)
	//UFUNCTION(BlueprintCallable, Category = "ItemManager")
	AC_ItemPickUp* SpawnItemPickUp(const FInventoryEntry& InEntry, const FVector& SpawnLocation);

	UFUNCTION(BlueprintCallable, Category = "ItemManager")
	AC_ItemPickUp* BP_SpawnItemPickUp(const FInventoryEntry& InEntry, const FVector& SpawnLocation)
	{
		return SpawnItemPickUp(InEntry, SpawnLocation);
	}

	// --- DropItemByPlayer 오버로딩 ---
	// [1] Entry 기반 드롭 (추천), 플레이어가 자신이 들고 있는 아이템을 마크처럼 레벨에 뱉어냄.(AC_ItemPickUp 형태로)
	bool DropItemByPlayer(const FInventoryEntry& InEntry, AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "ItemManager")
	bool BP_DropItemByPlayer(const FInventoryEntry& InEntry, AActor* InActor)
	{
		return DropItemByPlayer(InEntry, InActor);
	}

	// [2] Name + Count 기반 드롭 (필요 시 단순 드롭용)
	UFUNCTION(BlueprintCallable, Category = "ItemManager")
	bool DropItemByPlayer(FName InRowName, int32 InCount, AActor* InActor);
	
	// [플레이어 장착용] 무조건 인벤의 있는 아이템의 정보를 사용한다.
	UFUNCTION(BlueprintCallable, Category = "ItemManager")
	AC_WeaponBase* SpawnEquippedActor(FName InRowName, AActor* InOwner = nullptr, const FTransform& SpawnTransform = FTransform());

	//template <typename T>
	//T* SpawnEquippedActor(FName InRowName, const FTransform& SpawnTransform, AActor* InOwner = nullptr)
	//{
	//	return Cast<T>(SpawnEquippedActor(InRowName, SpawnTransform, InOwner));
	//}
	
public:
	
	// T           : 반환할 데이터 구조체 타입 (FItemData, FGunData...)
	// InTableType : 반환받고 싶은 아이템의 데이터 테이블 타입
	// InRowName   : 가져오고 싶은 데이터의 RowName 
	template <typename T>
	const T* GetItemData(EItemTableType InTableType, FName InRowName) const;
	
	
	// [블루프린트 전용] Generic/BlueprintCallable 래퍼 함수
	UFUNCTION(BlueprintCallable, Category = "ItemManager", meta = (DisplayName = "Get Item Data"))
	bool GetItemDataBP(EItemTableType InTableType, FName InRowName, FInstancedStruct& OutData);
    
private:
	// Enum 키값 기반 데이터 테이블 원본 포인터 반환 헬퍼
	const UDataTable* GetTargetTable(EItemTableType InTableType) const;
	
private:
	// 동기 로드 완료된 데이터 테이블 포인터 맵 (런타임 캐싱)
	UPROPERTY()
	TMap<EItemTableType, TObjectPtr<UDataTable>> CachedItemTables;
};

template <typename T>
const T* UC_ItemManager::GetItemData(EItemTableType InTableType, FName InRowName) const
{
	if (InRowName.IsNone()) return nullptr;

	const UDataTable* Table = GetTargetTable(InTableType);
	if (!Table) return nullptr;

	return Table->FindRow<T>(InRowName, TEXT("GetItemDataContext"));
}