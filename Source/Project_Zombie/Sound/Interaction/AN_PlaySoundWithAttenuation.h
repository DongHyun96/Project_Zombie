// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_PlaySoundWithAttenuation.generated.h"

class USoundBase;
class USoundAttenuation;

UCLASS()
class PROJECT_ZOMBIE_API UAN_PlaySoundWithAttenuation : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:

	// Notify 실행될 때 재생할 Sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> m_Sound;

	// 거리 감쇠 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundAttenuation> m_AttenuationSetting;
};
