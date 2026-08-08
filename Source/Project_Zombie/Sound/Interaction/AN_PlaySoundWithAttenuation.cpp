#include "Sound/Interaction/AN_PlaySoundWithAttenuation.h"

#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"

void UAN_PlaySoundWithAttenuation::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !m_Sound)
		return;

	UGameplayStatics::PlaySoundAtLocation(
		MeshComp,	
		m_Sound,
		MeshComp->GetComponentLocation(),
		1.0f,		// Volume 
		1.0f,		// Pitch
		0.0f,		// Start Time
		m_AttenuationSetting, // Attenuation Setting
		nullptr
	);
}
