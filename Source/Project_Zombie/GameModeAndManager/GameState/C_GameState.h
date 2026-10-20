// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "C_GameState.generated.h"

/**
 * 주의 : AGameState를 상속받으면 AGameModeBase와 호환이 되지 않는다함 (AGameStateBase를 상속받은 클래스여야함)
 */
UCLASS()
class PROJECT_ZOMBIE_API AC_GameState : public AGameStateBase
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;
	
private:
	
	UFUNCTION()
	void OnRep_CurStageIdx();

public:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/// <summary>
	/// <para> 서버 쪽 환경에서 해당 Setter 사용할 것 -> 사용 시점 (전투 Stage 끝났을 시 && 이겼을 때 새로이 CurStageIdx를 올려야 할 때) </para>
	/// <para> 클라 환경에서도 이 시점에 Replication을 통해, CurStageIdx를 받아 동기화 처리를 해준다 </para>
	/// <para> StageManager(GameInstanceSubsystem)에서의 Idx 동기화 불가하기 때문에 GameState의 Replication을 사용하는 것 </para>
	/// </summary>
	void SetCurStageIdx(uint8 _StageIdx);
	
protected:

	// 현재 Stage (직접 고를수도 있게?) -> 미정
	// 역시 클라의 경우 서버 환경의 CurStageIdx 상태를 따름
	UPROPERTY(ReplicatedUsing = OnRep_CurStageIdx, VisibleAnywhere, BlueprintReadOnly)
	int32 m_CurStageIdx = -1;
	
	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AC*/

};
