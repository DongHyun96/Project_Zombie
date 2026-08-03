// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_PointTower.generated.h"

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
	
	/// <summary>
	/// 이 거점 활성화 처리 (점령 가능하도록) / 서버 쪽에서만 실행할 것
	/// </summary>
	/// <param name="_ConquerAmount"> : 활성화시킬 때의 현재 ConquerAmount 초기값 
	/// (한 Sequence 내의 여러 거점이 있을 경우, Amount를 조정해서 활성화 처리할 것 </param>
	/// <returns> 서버 환경이 아니라면 활성화 x / return false </returns>
	bool Activate(float _ConquerAmount = 0.f);

private:

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Activate(float _ConquerAmount);
	
protected:
	
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
};
