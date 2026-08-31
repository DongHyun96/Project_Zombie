// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "GameFramework/PlayerState.h"
#include "Multi/C_InvenStructures.h"
#include "C_PlayerState.generated.h"

class AC_WeaponBase;
/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API AC_PlayerState : public APlayerState
{
	GENERATED_BODY()

	friend class AC_GameMode_GameLv;
	
public:
	
	AC_PlayerState();

private:
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	
	bool IsHost() const { return m_bIsHost; }
	
	virtual void CopyProperties(APlayerState* NewPlayerState);
	
public:
	// 스탯 데이터 입출력
	void SaveStatsToState(const TMap<FName, float>& InStats, const TMap<FName, uint8>& InGrades)
	{
		SavedStats = InStats;
		SavedStatGrades = InGrades;
	}
	const TMap<FName, float>& GetSavedStats() const { return SavedStats; }
	const TMap<FName, uint8>& GetSavedStatGrades() const { return SavedStatGrades; }
	
	TArray<AC_WeaponBase*> GetSavedWeapons() const { return SavedWeapons; }

	// 인벤토리 데이터 입출력
	void SaveInventoryToState(const TArray<FInventoryEntry>& InContainer) { SavedInventoryContainers = InContainer; }
	const TArray<FInventoryEntry>& GetSavedInventory() const { return SavedInventoryContainers; }
	
	void ClearSavedInventory() { SavedInventoryContainers.Empty(); }
	
	void ClearSavedWeapons() { SavedWeapons.Empty(); }
	
	void ClearSavedStats() { SavedStats.Empty(); }
	
	void ClearSavedStatGrades() { SavedStatGrades.Empty(); }
	
protected:
	
	// TODO : 의미 없어보임.
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool m_bIsHost{};
	
	UPROPERTY()
	TArray<FInventoryEntry> SavedInventoryContainers{};
	
	UPROPERTY()
	TArray<AC_WeaponBase*> SavedWeapons{};	
	
	UPROPERTY()
	TMap<FName, float> SavedStats{};

	UPROPERTY()
	TMap<FName, uint8> SavedStatGrades{};
};

