// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StatComponent/C_StatComponentBase.h"
#include "C_PlayerStatComponent.generated.h"


class AC_StatUpgradeStation;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_ZOMBIE_API UC_PlayerStatComponent : public UC_StatComponentBase
{
	GENERATED_BODY()

public:
	
	UC_PlayerStatComponent();

public:
	
	virtual void BeginPlay() override;
	
	void Server_RequestStatUpgrade(AC_StatUpgradeStation* InInteractableActor, const FName& UpStatName);
	
	void BindUpdateOtherPlayerHPBar();
	
	// 레벨 전환시 PlayerState로 부터 백업된 정보로 플레이어의 스탯을 초기화 하는 함수.
	virtual void LoadStatsFromBackup(const TMap<FName, float>& InStats, const TMap<FName, uint8>& InGrades) override;
private:
	
	virtual UScriptStruct* GetStatDataStruct() const override;

private:
	
	void UpdateOtherPlayerHPBar(float _Ratio);
	
private:

	UPROPERTY()
	class AC_BasicPlayer* m_OwnerPlayer{};
	
	UPROPERTY()
	class UC_GameMainHUD* m_MainHUD{};
	
};
