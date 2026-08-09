// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_FireSound.generated.h"

class USoundBase;

UCLASS()
class PROJECT_ZOMBIE_API UAN_FireSound : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* m_LocalSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* m_RemoteSound;
};
