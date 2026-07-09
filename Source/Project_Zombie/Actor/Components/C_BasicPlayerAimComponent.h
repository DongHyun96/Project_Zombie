// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_BasicPlayerAimComponent.generated.h"

UENUM(BlueprintType)
enum class EAimState : uint8
{
	None,       // 일반 지향 사격 시점
	Shoulder,   // 견착 시점(우클릭 홀드)
	ADS         // 정조준 시점 (우클릭 딸깍 - 아이언 사이트 / 배율 스코프)
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_BasicPlayerAimComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	AC_BasicPlayer* m_CurPlayer;

	// [Aim]
protected:
	// 카메라 상태값
	bool bIsAiming = false;

	// 카메라가 조준/해제 상태로 이동 중인지 여부
	bool bIsTransitioningCamera = false;

	// 메인 카메라 렌즈 배율 저장용
	float BaseFOV;

	// 메인 카메라 오프셋 저장용
	FVector BaseCameraOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Aim|ShoulderCam")
	float m_AimFOV = 60.f; // 조준 시 변경될 FOV (기본은 90)

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Aim|ShoulderCam")
	float m_AimSpeed = 10.f; // 조준 속도

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Aim|ShoulderCam")
	FVector	m_AimOffset = FVector(0.f, 40.f, 20.f); // 조준 시 카메라 위치 조정용

private:
	float m_RuntimeTargetFOV;
	FVector m_RuntimeTargetOffset;

protected:
	virtual void BeginPlay() override;

public:
	bool IsTransitioningCamera() { return bIsTransitioningCamera; }

public:
	// 조준 입력 시 호출될 함수
	void OnAimPressed();
	void OnAimReleased();

	// 매 프레임 카메라 시점을 에임으로 구동할 함수
	void UpdateCameraInterpolation(float DeltaTime);

public:
	UC_BasicPlayerAimComponent();

};
