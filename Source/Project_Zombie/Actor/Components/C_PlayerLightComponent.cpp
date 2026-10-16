#include "Actor/Components/C_PlayerLightComponent.h"
#include "Components/SpotLightComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"

UC_PlayerLightComponent::UC_PlayerLightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UC_PlayerLightComponent::BeginPlay()
{
	Super::BeginPlay();

    m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
    if (!m_OwnerPlayer || !m_SpotLight)
        return;

    m_SpotLight->SetIntensity(5000.f);
    m_SpotLight->SetAttenuationRadius(1500.f);
    m_SpotLight->SetInnerConeAngle(20.f);
    m_SpotLight->SetOuterConeAngle(40.f);
}


void UC_PlayerLightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!m_OwnerPlayer || !m_SpotLight)
        return;

    FRotator TargetRotation;

    if (m_OwnerPlayer->IsLocallyControlled())
    {
        TargetRotation = m_OwnerPlayer->GetControlRotation();
    }

    else
    {
        const FRotator BaseAimRotation = m_OwnerPlayer->GetBaseAimRotation();

        TargetRotation = FRotator(
            BaseAimRotation.Pitch,
            m_OwnerPlayer->GetReplicatedLightYaw(),
            0.f
        );
    }

    if (!m_SpotLight->GetComponentRotation().Equals(TargetRotation, 0.1f))
    {
        m_SpotLight->SetWorldRotation(TargetRotation);
    }
}

void UC_PlayerLightComponent::InitLight(USpotLightComponent* _SpotLight)
{
	m_SpotLight = _SpotLight;
}
