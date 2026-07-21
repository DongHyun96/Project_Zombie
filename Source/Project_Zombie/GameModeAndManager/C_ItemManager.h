#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GlobalData.h"
#include "C_ItemManager.generated.h"

class AC_ItemPickUp;
class UDataTable;

UCLASS()
class PROJECT_ZOMBIE_API UC_ItemManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
    
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// T           : 반환할 데이터 구조체 타입 (FItemData, FGunData...)
	// InTableType : 반환받고 싶은 아이템의 데이터 테이블 타입
	// InRowName   : 가져오고 싶은 데이터의 RowName 
	template <typename T>
	const T* GetItemData(EItemTableType InTableType, FName InRowName) const;

	UFUNCTION(BlueprintCallable, Category = "ItemManager")
	AC_ItemPickUp* SpawnItem(FName InRowName, int32 InCount, const FVector& SpawnLocation);
    
	UFUNCTION(BlueprintCallable, Category = "ItemManager")
	bool DropItemByPlayer(FName InRowName, int32 InCount, AActor* InActor);
    
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