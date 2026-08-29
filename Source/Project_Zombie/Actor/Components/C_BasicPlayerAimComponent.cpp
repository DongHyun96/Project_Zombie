

#include "C_BasicPlayerAimComponent.h"

#include "C_EquippedComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "../Character/Player/C_BasicPlayer.h"
#include "../ItemActor/Weapon/Gun/C_GunBase.h"
#include "Components/WidgetComponent.h"

#include "GameModeAndManager/C_UIManager.h"
#include "UI/MainHUD/CrosshairWidget/C_CrosshairWidget.h"
#include "UI/MainHUD/C_GameMainHUD.h"

UC_BasicPlayerAimComponent::UC_BasicPlayerAimComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	m_MuzzleAwareWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("MuzzleAwareWidgetComponent"));
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

	m_MuzzleAwareWidgetComponent->SetHiddenInGame(true);
}

void UC_BasicPlayerAimComponent::OnAimPressed()
{
	bIsAiming = true;
	bIsTransitioningCamera = true;

	// 조준 시 달리기 강제 중단
	if (m_CurPlayer->GetPlayerMoveState() == EPlayerPoseState::Sprint)
		m_CurPlayer->StopSprint();

	if (UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld()))
		MainHUD->GetCrosshairWidget()->ZoomIn();

}

void UC_BasicPlayerAimComponent::OnAimReleased()
{
	bIsAiming = false;
	bIsTransitioningCamera = true;

	if (m_CurPlayer)
		m_CurPlayer->SetPlayerMoveState(EPlayerPoseState::Walk);

	if (UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld()))
		MainHUD->GetCrosshairWidget()->ZoomOut();
}

void UC_BasicPlayerAimComponent::UpdateCameraInterpolation(float DeltaTime)
{
	if (!m_CurPlayer) return;

	// 조준 중 이면 1.0, 해제 상태면 0.0
	float TargetAlpha = bIsAiming ? 1.0f : 0.0f;

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
		
		PC->SetViewTargetWithBlend(m_CurPlayer, 0.15f);
		
		bIsTransitioningCamera = false;
		return;
	}

	if (m_AimSpeed <= 0.f) m_AimSpeed = 10.f;
	float BlendInTime = 1.f / m_AimSpeed;

	if (bIsTransitioningCamera)
	{
		if (bIsAiming)
		{
			if (PC->GetViewTarget() != m_CurPlayer)
			{
				PC->SetViewTargetWithBlend(m_CurPlayer, BlendInTime, EViewTargetBlendFunction::VTBlend_Cubic);
			}
			m_RuntimeTargetFOV = m_AimFOV;
			m_RuntimeTargetOffset = m_AimOffset;
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

void UC_BasicPlayerAimComponent::ToggleMuzzleAwareCrossHair(bool _Visible)
{
	m_MuzzleAwareWidgetComponent->SetHiddenInGame(!_Visible);
}

void UC_BasicPlayerAimComponent::UpdateMuzzleAwareCrossHairLocation(const FVector& _WorldLocation)
{
	m_MuzzleAwareWidgetComponent->SetWorldLocation(_WorldLocation);
}
