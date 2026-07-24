// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_PoseColliderHandlerComponent.generated.h"

class AC_BasicPlayer;
class UCapsuleComponent;
class USkeletalMeshComponent;
class UCharacterMovementComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPoseTransitionFinished, bool);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_ZOMBIE_API UC_PoseColliderHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UC_PoseColliderHandlerComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/// <summary>
	/// 자세 변경 요청
	/// </summary>
	/// <param name="_bIsCrouched"> true : 웅크리기 / false : 일어서기 </param>
	bool SetCrouched(bool _bIsCrouched);

	/// <summary>
	/// 결정된 자세로 전환 시작
	/// 서버와의 동기화를 위해 보정 후의 진짜 자세 전환은 여기서 처리...
	/// </summary>
	void StartCrouchTransition(bool _bIsCrouched);

	/// 안쓰는 함수
	/// <summary>
	///	원격 플레이어용 함수. 서버에서 리플리케이트된 자세를 적용만 하고 Root 이동은 하지 않음
	/// </summary>
	/// <param name="_bIsCrouched"></param>
	void ApplyRemotePose(bool _bIsCrouched);

	/// <summary>
	/// 현재 위치에서 일어설 수 있는지 확인
	/// </summary>
	bool CanStand() const;

	/// <summary>
	/// Capsule 크기 반경 중인지 반환
	/// </summary>
	bool IsTransitioning() const { return m_bIsTransitioning; }

	/// <summary>
	/// 현재 완료된 자세가 Crouch 인지 반환
	/// </summary>
	bool IsCrouched() const { return m_bIsCrouched; }
		

public:
	// 자세 전환 완료 델리게이트
	FOnPoseTransitionFinished OnPoseTransitionFinished;

private:
	// Capsule 높이 변경
	void ApplyCapsuleHalfHeight(float _NewHalfHeight);

	// 바닥 정보 다시 맞추기 
	// 계속 땅 밑으로 내려가버려서 추가
	void RefreshFloorInformation();

	// 자세 전환 완료
	void FinishTransition();


private:
	UPROPERTY()
	AC_BasicPlayer* m_Player;

	UPROPERTY()
	UCapsuleComponent* m_CapsuleComponent;

	UPROPERTY()
	USkeletalMeshComponent* m_MeshComponent;

	UPROPERTY()
	UCharacterMovementComponent* m_CharacterMovementComponent;


public:
	// 웅크렸을 때 Capsule Half Height
	UPROPERTY(EditAnywhere, Category = "Pose Collider")
	float m_CrouchHalfHeight;

	// 자세 전환에 걸리는 시간
	UPROPERTY(EditAnywhere, Category = "Pose Collider")
	float m_TransitionDuration;


private:

	// 서 있을 때 SkeletalMesh Relative Location
	FVector m_StandMeshRelativeLocation;

	// 서 있을 때 Capsule Radius
	float m_StandRadius;

	// 서 있을 때 Capsule Half Height
	float m_StandHalfHeight;


private:

	// 어느 자세로 바꿀건지?
	bool m_bTargetCrouched;

	// 현재 완료된 자세
	bool m_bIsCrouched;

	// 자세 전환 중?
	bool m_bIsTransitioning;
};
