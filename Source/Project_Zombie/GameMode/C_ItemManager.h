#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GlobalData.h"
#include "C_ItemManager.generated.h"


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
	class AC_ItemPickUp* SpawnItem(FName InRowName, const FVector& SpawnLocation, const FVector& LaunchVelocity = FVector::ZeroVector);
private:

	UPROPERTY()
	UDataTable* ItemDataTable = nullptr;
	
protected:

};
