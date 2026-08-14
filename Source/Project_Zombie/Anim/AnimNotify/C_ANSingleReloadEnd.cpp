// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ANSingleReloadEnd.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_EquippedComponent.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "Utility/C_Util.h"

class AC_GunBase;

void UC_ANSingleReloadEnd::Notify
(
	USkeletalMeshComponent*			 MeshComp,
	UAnimSequenceBase*				 Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!MeshComp)
	{
		UC_Util::Print("[UC_ANReloadEnd::Notify] : MeshComp nullptr", FColor::Red, 10.f);
		return;
	}
	
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(MeshComp->GetOwner());
	if (!Player)
	{
		UC_Util::Print("[UC_ANReloadEnd::Notify] : This AN is for PlayerMesh animation!", FColor::Red, 10.f);
		return;
	}

	AC_GunBase* Gun = Cast<AC_GunBase>(Player->GetEquippedComponent()->GetCurWeapon());
	if (!Gun)
	{
		UC_Util::Print("[UC_ANReloadEnd::Notify] : CurWeapon is not gun type!", FColor::Red, 10.f);
		return;
	}

	// 실질적인 재장전 이후의 처리를 이 AN에서 처리를 해줄 것 (Local의 LocallyControlled Character에 대해)
	Gun->AN_OnSingleReloadEnd();
	
	
}
