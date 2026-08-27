// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/FireSound/AN_FireSound.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"


void UAN_FireSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp)
		return;

	AC_BasicPlayer* OwnerPlayer = nullptr;

	if (AC_WeaponBase* OwnerWeapon = Cast<AC_WeaponBase>(MeshComp->GetOwner()))
	{
		OwnerPlayer = OwnerWeapon->GetOwnerPlayer();
	}
	else
	{
		OwnerPlayer = Cast<AC_BasicPlayer>(MeshComp->GetOwner());
	}

	if (OwnerPlayer && OwnerPlayer->IsLocallyControlled())
	{
		UGameplayStatics::PlaySound2D(MeshComp, m_LocalSound);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[FireSound] REMOTE"));

	UGameplayStatics::PlaySoundAtLocation(MeshComp, m_RemoteSound, MeshComp->GetComponentLocation());
}
