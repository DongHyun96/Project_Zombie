// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	
private:
	
	/// <summary>
	/// 현재 Sequence의 거점들 모두 활성화 
	/// </summary>
	void StartActivateCurrentPointsSequence();
	
public:
	
	/// <summary>
	/// PointTowerManager에 PointTower 등록 
	/// 등록과 동시에, 받은 PointTower의 Sequence보다 m_PointTowers 사이즈가 작다면, 사이즈 확보처리까지
	/// </summary>
	/// <param name="_PointTower"></param>
	/// <returns> : 제대로 등록 처리되었다면 return true </returns>
	bool RegisterPointTower(class AC_PointTower* _PointTower);

public:
	
	/// <summary>
	/// 현재 진행중인 Sequence의 PointTower 점령되었을 때 호출됨 -> 다음 Sequence로 넘어가야 하는지 체크 + 넘어가야 하는 상황이라면 다음 라운드 Sequence로 넘어감 
	/// </summary>
	void OnPointTowerConquered();
	
private:

	// 현재 활성화된 Sequence
	uint8 m_CurrentSequenceIndex{};
	
	// 각 Sequence 마다 활성화될 거점들 (한 Sequence 내에 여러 거점이 동시에 활성화될 수 있음)
	TArray<TSet<class AC_PointTower*>> m_PointTowers{};

private:
	
	FTimerHandle m_TestTimerHandle{};
	
};
