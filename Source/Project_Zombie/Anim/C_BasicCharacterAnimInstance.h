// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "C_BasicCharacterAnimInstance.generated.h"

struct FMontageBlendSettings;

/**
 * 게임 내의 모든 캐릭터 Anim Blueprint용 AnimInstance의 최상위 부모
 * AnimMontage 우선순위 처리 재생 기능 구현
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_BasicCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	/// <summary>
	/// <para> Montage_Play 함수 내에서 실질적인 재생 처리는 이 함수가 담당(Montage_Play는 virtual 함수가 아니기에, 이 함수를 override함) </para>
	/// <para> 재생 이전에, 현재 Play중인 Montage가 있다면, 서로 우선순위를 비교해서 새로 들어온 AnimMontage를 재생할지 말지 결정 </para>
	/// </summary>
	/// <returns> 재생 </returns>
	virtual float Montage_PlayInternal
	(
		UAnimMontage*					MontageToPlay,
		const FMontageBlendSettings&	BlendInSettings,
		float							InPlayRate				= 1,
		EMontagePlayReturnType			ReturnValueType			= EMontagePlayReturnType::MontageLength,
		float							InTimeToStartMontageAt	= 0,
		bool							bStopAllMontages		= true
	) override;

private:

	// <GroupName, AnimMontage> | 현재 재생 중인, 또는 직전에 재생한 PriorityAnimMontage 정보
	TMap<FName, UAnimMontage*> m_CurPriorityAnimMontage{};
};
