// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_FireDamageArea.generated.h"


// 각 장판의 위치와 Normal 정보를 담는 구조체
struct FFirePatchInfo
{
	FVector PatchLocation; // 장판 위치
	FVector PatchNormal;   // 장판이 바닥에 닿았을 때의 Normal
};

class USoundBase;
class USoundAttenuation;
class UAudioComponent;

UCLASS()
class PROJECT_ZOMBIE_API AC_FireDamageArea : public AActor
{
	GENERATED_BODY()
	
public:	
	AC_FireDamageArea();

protected:
	virtual void BeginPlay() override;

	// 장판이 끝날 때 호출
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	/// <summary>
	///	폭발 위치를 중심으로 화염 장판을 생성
	/// </summary>
	void GenerateFirePatches();


	/// <summary>
	/// 퍼져나가는 화염이 바닥에 닿는지 확인하고, 닿는다면 HitResult를 반환
	/// </summary>
	bool FindGroundAtLocation(const FVector& _PointLocation, FHitResult& _OutGroundHit);

	/// <summary>
	// 두 화염 사이에 장애물이 있는지 확인
	/// </summary>
	bool IsSpreadBlock(const FFirePatchInfo& _FromPatch, const FFirePatchInfo& _EndPatch);

	/// <summary>
	// 화염 장판 정보를 배열에 추가하고 파티클을 생성
	/// </summary>
	void AddFirePatch(const FFirePatchInfo& _Patch);

	/// <summary>
	// 대상이 화염 데미지 영역 안에 있는지 검사
	/// </summary>
	bool IsTargetInFireArea(const FVector& _TargetLocation);

	/// <summary>
	// 화염 데미지 영역에 들어온 액터에게 데미지를 적용
	/// </summary>
	void ApplyPointDamage();



protected:

	// =================== Effect 관련 변수 ===================

	// 장판이 바닥에 닿았을 때 생성되는 파티클
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Effect")
	TObjectPtr<UParticleSystem> m_FirePatchEffect;

	// 장판 파티클의 크기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Effect")
	float m_FirePatchEffectScale;

	// =================== Spread 관련 변수 ===================

	// 중심에서 퍼지는 방향의 개수 (360도 기준)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Spread")
	int32 m_SpreadDirectionCount;

	// 한 방향에서 퍼지는 장판 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Spread")
	int32 m_PatchDirectionCount;

	// 장판 간격
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Spread")
	float m_PatchSpacing;

	// 장판 데미지 영역 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Spread")
	float m_PatchDamageRadius;

	// ==================== Ground 관련 변수 ===================

	// 벽을 검사 Trace 를 바닥에서 띄우는 높이
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Ground")
	float m_ObstacleTraceHeight;

	// 바닥을 찾기 위한 Trace 거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Ground")
	float m_GroundTraceDistance;

	// 바닥으로 인한 Normal의 Z값이 이 값보다 작으면 벽이라고 판단		
	// 평평한 바닥 : 1 	수직 벽 : 0
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Ground")
	float m_GroundNormalZ;

	// ==================== Damage 관련 변수 ===================

	// 데미지 영역 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Damage")
	float m_DamageRadius;

	// 데미지 영역 높이 (Z축)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Damage")
	float m_DamageHalfHeight;

	// 장판 지속 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Damage")
	float m_Duration;

	// 초당 데미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Damage")
	float m_DamagePerSecond; 

	// 데미지를 적용하는 간격 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Damage")
	float m_DamageInterval; 

	// 데미지 타입
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Damage")
	TSubclassOf<UDamageType> m_DamageType;


	// ==================== Debug 관련 변수 ===================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Area|Debug")
	bool m_bDebugDraw; 

protected:
	// ==================== Sound 관련 변수 ===================
	UPROPERTY(EditDefaultsOnly, Category = "Fire Area|Sound")
	USoundBase* m_FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Fire Area|Sound")
	USoundAttenuation* m_FireSoundAttenuation;

	UPROPERTY()
	UAudioComponent* m_FireAudioComponent;

private:

	// 생성된 장판의 정보를 담는 배열
	TArray<FFirePatchInfo> m_FirePatchInfos;

	// 생성된 화염 파티클 컴포넌트들을 담는 배열
	UPROPERTY(Transient)
	TArray<TObjectPtr<UParticleSystemComponent>> m_FirePatchEffects;

	// 데미지 반복 타이머 핸들
	FTimerHandle m_DamageTimerHandle;
};
