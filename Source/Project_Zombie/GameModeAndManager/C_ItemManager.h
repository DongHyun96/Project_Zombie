#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GlobalData.h"
#include "C_ItemManager.generated.h"

class AC_ItemPickUp;

UCLASS(Blueprintable)
class PROJECT_ZOMBIE_API UC_ItemManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// FName 키값으로 아이템 데이터 포인터를 빠르게 반환하는 함수
	//UFUNCTION(BlueprintCallable)
	const FItemData* GetItemData(FName InRowName) const;

	UFUNCTION(BlueprintCallable)
	AC_ItemPickUp* SpawnItem(FName InRowName, int32 InCount, const FVector& SpawnLocation);
	
	UFUNCTION(BlueprintCallable)
	bool DropItemByPlayer(FName InRowName, int32 InCount, AActor* InActor);
private:

	UPROPERTY()
	UDataTable* ItemDataTable = nullptr;
	
protected:

};
