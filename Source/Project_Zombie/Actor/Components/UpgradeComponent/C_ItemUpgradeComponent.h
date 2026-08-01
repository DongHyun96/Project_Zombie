// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GlobalData.h"
#include "C_ItemUpgradeComponent.generated.h"

class UC_ItemManager;
class AC_BasicPlayer;

// 아이템을 강화하는 컴포넌트
// InteractableBase쪽에 붙어서 UpgradeWidget을 통해 클라가 서버에 강화를 요청하도록 할 예정.
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_ItemUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UC_ItemUpgradeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//bool CosumRequstItme()
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void UpgradeItem(AC_BasicPlayer* InPlayer, int32 InItemIndex, EUpgradableStats TargetStat);
	
private:
	// 사전 조건 검증 (아이템 유효성, 등급 제한 체크 등)
	bool ValidateUpgradeTarget(FInventoryEntry* InEntry, EUpgradableStats TargetStat, uint8& OutCurGrade);

	// 강화 재료 계산 및 소모 처리
	void ConsumeUpgradeMaterials(AC_BasicPlayer* InPlayer, UC_InvenComponent* InvenComp, FName ItemRowName, EUpgradableStats TargetStat, uint8 CurGrade);

	// 장착 중인 무기일 경우 실시간 데이터 갱신
	void UpdateEquippedWeaponData(AC_BasicPlayer* InPlayer, UC_ItemManager* ItemManager, FName ItemRowName, int32 InItemIndex);

	// 강화 로직 종료 및 클라이언트 알림
	void NotifyUpgradeFinished(AC_BasicPlayer* InPlayer);
};
