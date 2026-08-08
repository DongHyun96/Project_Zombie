// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_GameOverChecker.generated.h"

/// <summary>
/// 게임 오버 상황을 전파하는 Replicate 처리된 단순 Actor
/// 지금 시간이 없어서 GameOverChecker를 통해 RPC 호출을 못하는 UObject까지도 겸사겸사 다리로 그냥 쓰는 중 (에헤이 조졌네 이거)
/// </summary>
UCLASS()
class PROJECT_ZOMBIE_API AC_GameOverChecker : public AActor
{
	GENERATED_BODY()

public:
	
	AC_GameOverChecker();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

	/// <summary>
	/// 게임오버 시, 호출
	/// </summary>
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_GameOver(bool _PlayerWin);


	/// <summary>
	/// 
	/// </summary>
	/// <param name="_LeftTime"></param>
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateGameStartLeftTime(int32 _LeftTime);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowMainInformConqueringPointTower();
	
};
