// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_GameOverChecker.generated.h"

/// <summary>
/// 게임 오버 상황을 전파하는 Replicate 처리된 단순 Actor
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

	/// <summary>
	/// <para> 현재 Sequence가 새로 열렸고, PointTower 점령을 하라고 MainInform 멀티캐스트 처리 </para>
	/// <para> + InformWidget 좌상단 RemainTime 정보 및 PointTower 점령 정보 % 활성화 처리 </para>
	/// </summary>
	/// <param name="_CurSeqPointTowerCount"> : 현재 Sequence에 포함된 PointTower 개수 </param>
	/// <param name="_CurSeqRemainTime"> : 현재 Seq의 Remain Time 초기값 </param>
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowMainInformConqueringPointTower(uint8 _CurSeqPointTowerCount, int32 _CurSeqRemainTime);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateRemainTime(int32 _RemainTime);
	
public:
	
	/// <summary>
	/// 서버 쪽에서는 바로 정확한 GameOver 여부 확인 가능 / Client의 경우 Multicast RPC call을 받기 이전에 처리가 안되어 있을 수 있음(주의) 
	/// 애초에 서버 쪽에서만 GameOverChecker 객체를 현재 구할 수 있는 상황임
	/// </summary>
	bool HasGameOver() const { return m_bGameOver; }

private:
	
	bool m_bGameOver{};
	
	
};
