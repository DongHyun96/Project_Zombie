// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UI/MainHUD/CompassBarWidget/C_CompassBarWidget.h"
#include "UI/MainHUD/CompassBarWidget/CompassMarkerWidget/C_CompassMarkerWidget.h"
#include "C_PointTower.generated.h"

UENUM(BlueprintType)
enum class EPointTowerState : uint8
{
	Waiting,	// 아직 이 Tower가 활성화되기 이전 라운드인 경우
	Active,		// 이 Tower를 점령해야하는 Round이고, 거점이 활성화된 State
	Conquered	// 활성화된 거점을 Player 팀의 interaction 처리로 점령한 상황
};

/// <summary>
/// 주의 : 인게임 레벨에 배치 시, EditInstanceOnly 되어있는 멤버변수 초기화 시켜줄 것 (어떤 값인지 주석 확인할 것)
/// </summary>
UCLASS()
class PROJECT_ZOMBIE_API AC_PointTower : public AActor
{
	GENERATED_BODY()

	friend class UC_PointTowerManager;

public:
	AC_PointTower();

protected:
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

public:

	float GetZombieDamageRatio() const { return m_ZombieDamageRatio; }
	
public:

	void SetPointTowerState(EPointTowerState _PointTowerState);
	
	void TestFunction();
	void TestFunction2();
	
private:
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Activate();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Conquered();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateConquerAmountInt(uint8 _CurrentConquerAmount);
	
private:
	
	/// <summary>
	/// Active한 거점의 경우, Local player가 근접하면 Generator만 Outline 표시 처리 
	/// </summary>
	UFUNCTION()
	void OnApproachEffectTogglerColliderBeginOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor*				 OtherActor,
		UPrimitiveComponent* OtherComp,
		int32				 OtherBodyIndex,
		bool			  	 bFromSweep,
		const FHitResult& 	 SweepResult
	);

	/// <summary>
	/// Active한 거점의 경우, Local Player가 빠져나가면 모두 Outline 표시 처리 
	/// </summary>
	UFUNCTION()
	void OnApproachEffectTogglerColliderEndOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor*				 OtherActor,
		UPrimitiveComponent* OtherComp,
		int32				 OtherBodyIndex
	);

private:
	
	
	
private:
	
	USceneComponent* FindSceneComponentByName(const FName& _ComName);
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EPointTowerState m_State{};
	
	// 점령 게이지 value 최대량 (이건 100으로 두고, 거점 점령 난이도 조절이 필요할 시, 거점 활성화 속도값을 건드릴 것)
	static const float m_MaxConquerAmount;

	// 현재 점령한 점령 게이지 (서버 쪽에서만 유효)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float m_CurConquerAmount = 50.f; // TODO : 이 값 다시 0으로 세팅해줄 것
	
	// 현재 점령한 점령 게이지 int값 (이걸로 실시간 게이지량을 맞춤 -> 매 Tick마다 RPC Call이나 Replicate 처리는 무거움)
	// UI Display 처리 또한 이 값으로 진행한다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 m_CurConquerAmountInt{};

	// 몇 번째 Sequence로 거점 점령 활성화 처리가 되는지 (거점 점령 Activate 처리가 되는지)
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	uint8 m_ActivateSequenceIdx{};

	// 거점 Activate 상태 시, 기본 초당 거점 게이지 하락량(초당) -> 기본값 0.5로 세팅해둠
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	float m_DefaultDecreasingAmountOfConquerAmountPerSec = 0.5f;

	// 거점 Activate 상태 시, Player 한 명(오로지 한 명이어야 한다)이 활성화 처리를 할 때, 초당 거점 게이지 오르는 양
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	float m_IncreaseAmountPerSec = 5.f;
	
	// 거점 Activate 상태 시, Player 한 명(오로지 한 명이어야 한다)이 활성화 처리를 할 때, 초당 해당 Player에게 주는 Damage량
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	float m_DPSWhileConquering = 5.f;

	// 좀비에게 공격당했을 시, 좀비 공격력 x Ratio(해당 변수) 만큼 거점 게이지 즉각 하락 처리
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	float m_ZombieDamageRatio = 0.25f;

	// 현재 Active 상태에서 Conquering 중인 Player -> 얘의 Damage 처리를 주는 처리를 어떤식으로 해야할지...
	UPROPERTY()
	AC_BasicPlayer* m_ConqueringPlayer{};
	
private:
	
	// 거점 활성화(점령) 완료 이후로도 좀비에게 공격을 당할 수 있는지
	// 기본적으로 false
	// 마지막 타워 2개의 경우, 2개를 동시에 공략해야되게끔 2개를 한 번에 점령하기 위한 setting 값
	bool m_bCanDamagedAfterConquer{};
	
protected:

	UPROPERTY()
	class AC_WorldPingActor* m_WorldPingActor{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AC_WorldPingActor> m_WorldPingActorClass{};

	// Activate 상태 시, 본인이 활성화시킨 CompassMarkerWidget
	UC_CompassMarkerWidget* m_ActivatedCompassMarkerWidget{};
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMeshComponent* m_StaticMeshComTower{}; // 타워 본체 자체 StaticMeshCom

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMeshComponent* m_StaticMeshComGenerator{}; // 타워 옆에 놓인 발전기 (이 발전기 주변에서 Interaction 처리를 할 것임)

protected:

	// 플레이어가 적정거리 근접했을 때, Activate 상황이라면 시인성을 위한 Effect 켜고 끄는 처리 담당 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class USphereComponent* m_ApproachEffectTogglerCollider{};

private:

	// Electronic 이펙트들 부모
	UPROPERTY()
	USceneComponent* m_ElectroSplinesParent{};

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UC_PointTowerWidget* m_PointTowerWidget{};
	
	
private:
	
	FTimerHandle m_TestTimerHandle{};
	FTimerHandle m_TestTimerHandle2{};
	
};
