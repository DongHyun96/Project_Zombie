// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "C_ANSingleReloadEnd.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_ANSingleReloadEnd : public UAnimNotify
{
	GENERATED_BODY()

public:

	virtual void Notify
	(
		USkeletalMeshComponent* 		 MeshComp,
		UAnimSequenceBase*				 Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
	
};
