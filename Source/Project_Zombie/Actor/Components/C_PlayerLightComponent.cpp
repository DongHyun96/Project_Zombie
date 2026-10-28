#include "Actor/Components/C_PlayerLightComponent.h"
#include "Components/SpotLightComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/ChapterManager/C_StageManager.h"
#include "Kismet/GameplayStatics.h"

UC_PlayerLightComponent::UC_PlayerLightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UC_PlayerLightComponent::BeginPlay()
{
	Super::BeginPlay();

    m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
    if (!m_OwnerPlayer || !m_SpotLight)
    {
        PRINT_LOCAL(GetWorld(), "[UC_PlayerLightComponent::BeginPlay] : OwnerPlayer or SpotLight nullptr", FColor::Red, 10.f);
        return;
    }
    
    // m_SpotLight->SetIntensity(5000.f);
    // m_SpotLight->SetAttenuationRadius(1500.f);
    // m_SpotLight->SetInnerConeAngle(20.f);
    // m_SpotLight->SetOuterConeAngle(40.f);
    
    // Lobby Level이 아닌 경우 && TimeOfDay가 Night인 경우에만 LightComponent 활성화
    const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
    if (!CurrentLevelName.Contains(TEXT("Lobby")) && !CurrentLevelName.Contains(TEXT("Loby"))) // Typo로 'Loby'로 되어있음
    {
        UC_StageManager* StageManager = m_OwnerPlayer->GetGameInstance()->GetSubsystem<UC_StageManager>();
        
        const FGameStageInfo* CurStageInfo = StageManager->GetCurStageInfo();
        
        if (!CurStageInfo)  ToggleLight(false);
        else                ToggleLight(CurStageInfo->TimeOfDay == EStageTimeOfDay::Night); // Night인 경우에만 Light 활성화
    }
        
}


void UC_PlayerLightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!m_bEnabled) return;
    
    if (!m_OwnerPlayer || !m_SpotLight)
        return;

    FRotator TargetRotation;

    if (m_OwnerPlayer->IsLocallyControlled())
        TargetRotation = m_OwnerPlayer->GetControlRotation();
    else
    {
        const FRotator BaseAimRotation = m_OwnerPlayer->GetBaseAimRotation();

        TargetRotation = FRotator
        (
            BaseAimRotation.Pitch,
            m_OwnerPlayer->GetReplicatedLightYaw(),
            0.f
        );
    }
    
    // 캐릭터의 실제 정면 방향을 기준으로 뒤로 넘어가지는 않게끔 조정
    const float ActorYaw = m_OwnerPlayer->GetActorRotation().Yaw;
    
    // 캐릭터 정면과 불빛 방향 사이의 상대 각도
    const float RelativeYaw            = FMath::FindDeltaAngleDegrees(ActorYaw, TargetRotation.Yaw);
    constexpr float MaxLightYaw        = 90.f;
    const float ClampedRelativeYawDest = FMath::Clamp(RelativeYaw, -MaxLightYaw, MaxLightYaw);
    
    // -90 -> 90 바로 넘어가면 이상 -> 따라서 Lerp로 처리함
    m_ClampedRelativeYaw = FMath::Lerp(m_ClampedRelativeYaw, ClampedRelativeYawDest, 15.f * DeltaTime);
    TargetRotation.Yaw   = ActorYaw + m_ClampedRelativeYaw;

    if (!m_SpotLight->GetComponentRotation().Equals(TargetRotation, 0.1f))
        m_SpotLight->SetWorldRotation(TargetRotation);
}

void UC_PlayerLightComponent::InitLight(USpotLightComponent* _SpotLight)
{
	m_SpotLight = _SpotLight;
}

void UC_PlayerLightComponent::ToggleLight(bool _Enabled)
{
    m_bEnabled = _Enabled;
    if (m_SpotLight) m_SpotLight->SetHiddenInGame(!_Enabled);
}
