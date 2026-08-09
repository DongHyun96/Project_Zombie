// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "UObject/Object.h"
#include "C_PointTowerManager.generated.h"

/**
 *  거점 Sequence에 맞추어, 이번 거점 활성화 처리 및 다음 거점 활성화 처리 등을 도맡음
 *  서버 환경에서만 생성할 예정이고, 거점 활성화 Sequence를 다룰 예정
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_PointTowerManager : public UObject
{
	GENERATED_BODY()

public:
	
	UC_PointTowerManager();

public:
	
	/// <summary>
	/// GameMode BeginPlay 시점에 호출됨 
	/// </summary>
	void OnWorldBeginPlay();

	/// <returns> Tick이 더이상 필요 없다면 return false </returns>
	bool WorldTick(float _DeltaTime);
	
private:
	
	/// <summary>
	/// 현재 Sequence의 거점들 모두 활성화 
	/// </summary>
	void StartActivateCurrentPointsSequence();

	/// <summary>
	/// 현재 Sequence의 PointTower에서 
	/// 해당 Sequence의 WaveSetting을 가져옴
	/// </summary>
	const  FZombieWaveSetting* GetCurrentWaveSetting() const;
	
public:
	
	/// <summary>
	/// PointTowerManager에 PointTower 등록 
	/// 등록과 동시에, 받은 PointTower의 Sequence보다 m_PointTowers 사이즈가 작다면, 사이즈 확보처리까지
	/// </summary>
	/// <param name="_PointTower"></param>
	/// <returns> : 제대로 등록 처리되었다면 return true </returns>
	bool RegisterPointTower(class AC_PointTower* _PointTower);

	uint8 GetCurrentSequenceIdx() const { return m_CurrentSequenceIndex; }
	
	const TSet<AC_PointTower*>& GetCurPointTowers() const;
	
public:
	
	/// <summary>
	/// 현재 진행중인 Sequence의 PointTower 점령되었을 때 호출됨 -> 다음 Sequence로 넘어가야 하는지 체크 + 넘어가야 하는 상황이라면 다음 라운드 Sequence로 넘어감 
	/// </summary>
	void OnPointTowerConquered();

public:

	/// <summary>
	/// SpawnArea를 해당 Sequence에 등록
	/// </summary>
	bool RegisterSpawnArea(class AC_SpawnArea* _SpawnArea);

private:

	/// <summary>
	/// 현재 Sequence에 등록된 SpawnArea들을 반환 
	/// </summary>
	TArray<AC_SpawnArea*> GetCurrentSequenceSpawnAreas() const;
	
public:
	void SetZombieManager(class UC_ZombieManager* _ZombieManager) { m_ZombieManager = _ZombieManager; }

private:

	// 현재 활성화된 Sequence
	uint8 m_CurrentSequenceIndex{};
	
	// 각 Sequence 마다 활성화될 거점들 (한 Sequence 내에 여러 거점이 동시에 활성화될 수 있음)
	TArray<TSet<class AC_PointTower*>> m_PointTowers{};

	// Sequence 별 SpawnArea
	TArray<TSet<class AC_SpawnArea*>> m_SpawnArea;

	UPROPERTY()
	TSet<AC_PointTower*> m_Dummy{};


private:
	
	FTimerHandle m_FirstPointOpenWaitTimerHandle{};

	UPROPERTY()
	TObjectPtr<class UC_ZombieManager> m_ZombieManager;
	
private:

	// 일단 20초로 두어봤음 (만약 더 유하게 다른 플레이어들 접속을 기다려야 한다면 값 늘릴 것)
	const float m_GameStartWaitTime = 20.f;
	
	// 현재 게임 시작 기다리기까지 남은 초 -> 이걸로 실시간 동기화 처리할 것
	// UI Display 처리 또한 이 값으로 진행
	int32 m_GameStartTimeLeftInt{};
	
private:
	
	bool m_GameStartTimerSet{};
	
};
