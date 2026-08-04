// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
	
	bool IsConquered() const { return m_CurConquerAmount >= m_MaxConquerAmount; }
	
public:
	
	void SetPointTowerState(EPointTowerState _PointTowerState);
	
	/// <summary>
	/// 이 거점 활성화 처리 (점령 가능한 거점으로 활성화) / 서버 쪽에서만 실행할 것
	/// </summary>
	/// <param name="_ActivatedConquerAmount"> : 활성화시킬 때의 현재 ConquerAmount 초기값 
	/// (한 Sequence 내의 여러 거점이 있을 경우, Amount를 조정해서 활성화 처리할 것 </param>
	/// <returns> 서버 환경이 아니라면 활성화 x / return false </returns>
	bool Activate(float _ActivatedConquerAmount = 0.f);

	void TestFunction();
	
private:
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Activate();

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
	
protected:

	// 거점 점령 활성화된 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool m_bIsActive{};
	
	EPointTowerState m_State{};
	
	// 점령 게이지 value 최대량 (이건 100으로 두고, 거점 점령 난이도 조절이 필요할 시, 거점 활성화 속도값을 건드릴 것)
	static const float m_MaxConquerAmount;

	// 현재 점령한 점령 게이지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float m_CurConquerAmount{};

	// 몇 번째 Sequence로 거점 점령 활성화 처리가 되는지 (거점 점령 Activate 처리가 되는지)
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	uint8 m_ActivateSequenceIdx{};

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
	
	FTimerHandle m_TestTimerHandle{};
	
};
