// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_SpawnArea.generated.h"

// 스폰 박스 및 스폰 위치 정보 클래스

enum class EZombieType : uint8;

UCLASS()
class PROJECT_ZOMBIE_API AC_SpawnArea : public AActor
{
	GENERATED_BODY()

protected: /* 컴포넌트 */

	// 컴포넌트를 붙일 기본루트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> m_Root;

	// 레벨에서 좀비 스폰 범위를 표시하는 박스
	// 스폰 위치 계산 용도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UBoxComponent> m_SpawnBox;

protected: /* 스폰 세팅 */

	// 현재 해당 SpawnArea를 사용할 수 있는지
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spawn")
	bool m_bEnabled = true;

	// 이 영역에서 스폰을 허용할 좀비 타입
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spawn")
	TArray<EZombieType> m_AllowedZombieType;

	// 한 번의 요청에서 유효한 위치를 찾기위해 반복할 최대 횟수
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "1", UMin = "1"))
	int32 m_MaxSpawnAttempts = 15;

	// NavMesh 위로 위치를 찾을 때 사용할 탐색 범위
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spawn|Navigation")
	FVector m_NavProjectionExtent = FVector(150.f, 150.f, 500.f);

	// 바닥과 캡슐이 너무 정확히 붙어서 겹침 판정이 나는 것을 방지하는 여유 높이
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spawn|Collision", meta = (ClampMin = "0.0"))
	float m_GroundOffset = 2.f;

public:

	/// <summary>
	/// 현재 SpawnArea가 활성화되어 있는지 반환
	/// </summary>
	bool IsEnabled() const { return m_bEnabled; }

	/// <summary>
	/// SpawnArea 활성 여부 변경
	/// 거점 활성화/완료 시 사용 가능
	/// </summary>
	void SetEnabled(bool _Enabled) { m_bEnabled = _Enabled; }

	/// <summary>
	/// 전달받은 좀비 타입이 이 영역에서 스폰 가능한지 확인
	/// </summary>
	bool IsZombieTypeAllowed(EZombieType _ZombieType) const;

	/// <summary>
	/// 박스 내부에서 NavMesh와 장애물 검사를 통과하는
	/// 안전한 SpawnTransform을 찾아 반환
	/// </summary>
	/// <param name="_ZombieType">
	/// 스폰하려는 좀비 타입
	/// </param>
	/// <param name="_CapsuleRadius">
	/// 해당 좀비 캡슐 반지름
	/// </param>
	/// <param name="_CapsuleHalfHeight">
	/// 해당 좀비 캡슐 반높이
	/// </param>
	/// <param name="_OutTransform">
	/// 성공했을 때 반환되는 최종 스폰 위치와 회전
	/// </param>
	/// <returns>
	/// 유효한 위치를 찾았다면 true
	/// </returns>
	bool FindValidSpawnTransform
	(
		EZombieType		_ZombieType,
		float			_CapsuleRadius,
		float			_CapsuleHalfHeight,
		FTransform& _OutTransform
	) const;

protected:

	/// <summary>
	/// SpawnBox의 로컬 범위 안에서 무작위 월드 위치 생성
	/// 아직 NavMesh나 장애물 검사는 하지 않은 후보 위치
	/// </summary>
	FVector GetRandomPointInSpawnBox() const;

	/// <summary>
	/// NavMesh 보정된 위치가 SpawnBox의 XY 범위 안에
	/// 여전히 포함되는지 확인
	/// </summary>
	bool IsPointInsideSpawnBoxXY(const FVector& _WorldLocation) const;

	/// <summary>
	/// 후보 위치에 좀비 캡슐을 배치했을 때
	/// 벽, 지형, 다른 Pawn 등과 겹치는지 확인
	/// </summary>
	bool IsSpawnLocationBlocked
	(
		const FVector& _CapsuleCenter,
		float			_CapsuleRadius,
		float			_CapsuleHalfHeight
	) const;

public:	
	AC_SpawnArea();

};
