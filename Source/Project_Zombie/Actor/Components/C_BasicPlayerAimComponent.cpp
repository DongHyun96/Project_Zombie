

#include "C_BasicPlayerAimComponent.h"

#include "C_EquippedComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "../Character/Player/C_BasicPlayer.h"
#include "../ItemActor/Weapon/Gun/C_GunBase.h"


UC_BasicPlayerAimComponent::UC_BasicPlayerAimComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UC_BasicPlayerAimComponent::BeginPlay()
{
	Super::BeginPlay();

	m_CurPlayer = Cast<AC_BasicPlayer>(GetOwner());

	if (m_CurPlayer->GetCamera())
	{
		BaseFOV = m_CurPlayer->GetCamera()->FieldOfView;
	}

	if (m_CurPlayer->GetSpringArm())
	{
		BaseCameraOffset = m_CurPlayer->GetSpringArm()->SocketOffset;
	}

}

void UC_BasicPlayerAimComponent::OnAimPressed()
{
	bIsAiming = true;
	bIsTransitioningCamera = true;
}

void UC_BasicPlayerAimComponent::OnAimReleased()
{
	bIsAiming = false;
	bIsTransitioningCamera = true;
}

void UC_BasicPlayerAimComponent::UpdateCameraInterpolation(float DeltaTime)
{

	AC_GunBase* GunBase = Cast<AC_GunBase>(m_CurPlayer->GetEquippedComponent()->GetCurWeapon());

	// 무기 데이터 체크
	if (!GunBase)
	{
		bIsTransitioningCamera = false;
		return;
	}

	if (bIsAiming)
	{
		m_RuntimeTargetFOV = m_AimFOV;        // BP에서 적은 60.f
		m_RuntimeTargetOffset = m_AimOffset;  // BP에서 적은 오프셋
	}
	else
	{
		m_RuntimeTargetFOV = BaseFOV;         // BeginPlay 때 백업한 FOV (90)
		m_RuntimeTargetOffset = BaseCameraOffset; // 백업한 순정 오프셋
	}

	// 0 방지 예외 처리
	if (m_AimSpeed <= 0.f) m_AimSpeed = 10.f;

	//  선형 보간
	m_CurPlayer->GetCamera()->FieldOfView = FMath::FInterpTo(m_CurPlayer->GetCamera()->FieldOfView, m_RuntimeTargetFOV, DeltaTime, m_AimSpeed);
	m_CurPlayer->GetSpringArm()->SocketOffset = FMath::VInterpTo(m_CurPlayer->GetSpringArm()->SocketOffset, m_RuntimeTargetOffset, DeltaTime, m_AimSpeed);

	// 도달했는지 체크
	bool bFOVFinished = FMath::IsNearlyEqual(m_CurPlayer->GetCamera()->FieldOfView, m_RuntimeTargetFOV, 0.1f);
	bool bOffsetFinished = m_CurPlayer->GetSpringArm()->SocketOffset.Equals(m_RuntimeTargetOffset, 0.5f);

	if (bFOVFinished && bOffsetFinished)
	{
		// 최종 고정 및 틱 종료
		m_CurPlayer->SetCameraFOV(m_RuntimeTargetFOV);
		m_CurPlayer->SetSpringArmSocketOffset(m_RuntimeTargetOffset);
		bIsTransitioningCamera = false;
	}
}