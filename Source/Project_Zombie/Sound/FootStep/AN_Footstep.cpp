// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/FootStep/AN_Footstep.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_FeetComponent.h"

void UAN_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
		return;

	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(MeshComp->GetOwner());
	if (!Player)
		return;

	UC_FeetComponent* FeetComponent = Player->GetFeetComponent();
	if (!FeetComponent)
		return;

	FeetComponent->PlayFootstep(m_IsLeftFoot);
}
