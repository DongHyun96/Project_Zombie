

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

	if (m_CurPlayer)
	{

		if (m_CurPlayer->GetCamera())
		{
			BaseFOV = m_CurPlayer->GetCamera()->FieldOfView;
		}

		if (m_CurPlayer->GetSpringArm())
		{
			BaseCameraOffset = m_CurPlayer->GetSpringArm()->SocketOffset;
		}
	}

}

void UC_BasicPlayerAimComponent::OnAimPressed(EAimState TargetState)
{
	bIsAiming = true;
	bIsTransitioningCamera = true;
	m_CurAimState = TargetState;
}

void UC_BasicPlayerAimComponent::OnAimReleased()
{
	bIsAiming = false;
	bIsTransitioningCamera = true;
	m_CurAimState = EAimState::None;
}

void UC_BasicPlayerAimComponent::UpdateCameraInterpolation(float DeltaTime)
{
	if (!m_CurPlayer) return;

	// 조준 중 이면 1.0, 해제 상태면 0.0
	float TargetAlpha = (bIsAiming && m_CurAimState != EAimState::None) ? 1.0f : 0.0f;

	m_HandIKAlpha = FMath::FInterpTo(m_HandIKAlpha, TargetAlpha, DeltaTime, m_AimSpeed);

	APlayerController* PC = Cast<APlayerController>(m_CurPlayer->GetController());
	if (!PC) return;

	AC_GunBase* GunBase = nullptr;
	if (UC_EquippedComponent* EquipComp = m_CurPlayer->GetEquippedComponent())
	{
		GunBase = Cast<AC_GunBase>(EquipComp->GetCurWeapon());
	}

	if (!GunBase)
	{
		if (m_CurAimState != EAimState::None)
		{
			PC->SetViewTargetWithBlend(m_CurPlayer, 0.15f);
			m_CurAimState = EAimState::None;
		}
		bIsTransitioningCamera = false;
		return;
	}

	if (m_AimSpeed <= 0.f) m_AimSpeed = 10.f;
	float BlendInTime = 1.f / m_AimSpeed;

	// [1] 상태가 변한 첫 프레임에만 목적지 설정 및 시점 전환
	if (bIsTransitioningCamera)
	{
		if (bIsAiming)
		{
			if (m_CurAimState == EAimState::ADS)
			{
				PC->SetViewTargetWithBlend(GunBase, BlendInTime, EViewTargetBlendFunction::VTBlend_Cubic);
				bIsTransitioningCamera = false;
				return;
			}
			else if (m_CurAimState == EAimState::Shoulder)
			{
				// [견착] 플레이어 본인 시점인지 확인 후 목적지 설정 (스위치는 끄지 않음!)
				if (PC->GetViewTarget() != m_CurPlayer)
				{
					PC->SetViewTargetWithBlend(m_CurPlayer, BlendInTime, EViewTargetBlendFunction::VTBlend_Cubic);
				}
				m_RuntimeTargetFOV = m_AimFOV;
				m_RuntimeTargetOffset = m_AimOffset;
			}
		}
		else
		{
			// [조준 해제] 원래 캐릭터 시점으로 복귀 명령 및 순정 수치 목적지 설정
			if (PC->GetViewTarget() != m_CurPlayer)
			{
				PC->SetViewTargetWithBlend(m_CurPlayer, BlendInTime, EViewTargetBlendFunction::VTBlend_Cubic);
			}
			m_RuntimeTargetFOV = BaseFOV;
			m_RuntimeTargetOffset = BaseCameraOffset;
		}
	}

	// ADS 상태로 조준 중일 때는 메인 카메라 보간 연산을 아예 수행하지 않음
	if (m_CurAimState == EAimState::ADS && bIsAiming)
	{
		return;
	}

	// [2] 원래 잘 작동하던 부드러운 카메라 보간 및 도달 체크 (처음 올려주신 순정 로직)
	UCameraComponent* PlayerCam = m_CurPlayer->GetCamera();
	USpringArmComponent* SpringArm = m_CurPlayer->GetSpringArm();

	if (PlayerCam && SpringArm)
	{
		PlayerCam->FieldOfView = FMath::FInterpTo(PlayerCam->FieldOfView, m_RuntimeTargetFOV, DeltaTime, m_AimSpeed);
		SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, m_RuntimeTargetOffset, DeltaTime, m_AimSpeed);

		bool bFOVFinished = FMath::IsNearlyEqual(PlayerCam->FieldOfView, m_RuntimeTargetFOV, 0.1f);
		bool bOffsetFinished = SpringArm->SocketOffset.Equals(m_RuntimeTargetOffset, 0.5f);

		// 목적지에 도달했을 때만 안전하게 트랜지션 스위치를 끕니다.
		if (bFOVFinished && bOffsetFinished)
		{
			m_CurPlayer->SetCameraFOV(m_RuntimeTargetFOV);
			m_CurPlayer->SetSpringArmSocketOffset(m_RuntimeTargetOffset);
			bIsTransitioningCamera = false;
		}
	}
}