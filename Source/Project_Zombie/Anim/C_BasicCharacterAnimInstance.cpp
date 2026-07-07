// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BasicCharacterAnimInstance.h"
#include "AnimMontagePriority/C_MontagePriorityMetaData.h"
#include "DevloperSetting/C_MontagePrioritySettings.h"
#include "Utility/C_Util.h"

float UC_BasicCharacterAnimInstance::Montage_PlayInternal
(
	UAnimMontage*					MontageToPlay,
	const FMontageBlendSettings&	BlendInSettings,
	float							InPlayRate,
	EMontagePlayReturnType			ReturnValueType,
	float							InTimeToStartMontageAt, 
	bool							bStopAllMontages
)
{
	if (!MontageToPlay) return 0.f;

	const FName TargetGroup = MontageToPlay->GetGroupName();
	
	UAnimMontage** TargetGroupCurMontage = m_CurPriorityAnimMontage.Find(TargetGroup);
	
	// 자신의 Group내의 AnimMontage가 한번도 재생된 적 없을 땐 바로 재생
	if (!TargetGroupCurMontage)
	{
		m_CurPriorityAnimMontage.Add(TargetGroup, MontageToPlay);
		return Super::Montage_PlayInternal(MontageToPlay, BlendInSettings, InPlayRate, ReturnValueType, InTimeToStartMontageAt, bStopAllMontages);
	}

	// 직전의 AnimMontage의 재생이 이미 끝났을 때
	if (!Montage_IsPlaying(*TargetGroupCurMontage))
	{
		m_CurPriorityAnimMontage[TargetGroup] = MontageToPlay;
		return Super::Montage_PlayInternal(MontageToPlay, BlendInSettings, InPlayRate, ReturnValueType, InTimeToStartMontageAt, bStopAllMontages);
	}
	
	
	/* 
	 * 현재 같은 Group 내에서 재생중인 PriorityAnimMontage가 있을 때
	 * IncomingMontage vs CurPlayingMontage by Priority
	 * Priority를 비교해서 현재 Priority보다 크거나 같은 Priority라면, 새로 들어온 AnimMontage 재생
	 */
	
	
	UC_MontagePriorityMetaData* IncomingPriorityMetaData = MontageToPlay->FindMetaDataByClass<UC_MontagePriorityMetaData>();
	if (!IncomingPriorityMetaData) // 현재 들어온 Montage에 Priority 정보가 setting 되어있지 않으면 PriorityMax로 간주 -> 무조건 재생 처리한다.
	{
		m_CurPriorityAnimMontage.Add(TargetGroup, MontageToPlay);
		return Super::Montage_PlayInternal(MontageToPlay, BlendInSettings, InPlayRate, ReturnValueType, InTimeToStartMontageAt, bStopAllMontages);
	}
	
	UC_MontagePriorityMetaData* CurPlayingMetaData = (*TargetGroupCurMontage)->FindMetaDataByClass<UC_MontagePriorityMetaData>();
	
	// 이미 재생중인 Montage에 Priority 정보가 setting되어있지 않으면, 역시 PriorityMax로 간주 -> 무조건 이 Montage를 우선적으로 계속해서 재생처리한다.
	if (!CurPlayingMetaData) return 0.f; // 이번에 들어온 재생요청 씹어버리고, 기존 재생중이던 Montage 계속해서 재생 처리 
	
	
	const FGameplayTag& IncomingPriorityTag   = IncomingPriorityMetaData->GetMontagePriorityTag();	// 새로이 들어온 Montage의 PriorityTag
	const FGameplayTag& CurPlayingPriorityTag = CurPlayingMetaData->GetMontagePriorityTag();		// 이미 재생중인 Montage의 PriorityTag 
	
	const UC_MontagePrioritySettings* m_MontagePriorityProjectSettings = GetDefault<UC_MontagePrioritySettings>();

	uint8 IncomingPriority{};
	uint8 CurPlayingPriority{};
	
	/* 해당 Montage에 PriorityMontage 태그를 붙였는데, 잘못된 태그를 부여했을 경우에 대한 예외처리들 */
	if (!m_MontagePriorityProjectSettings->GetPriority(IncomingPriorityTag, IncomingPriority))
	{
		const FString& IncomingMontageName = MontageToPlay->GetName();
		UC_Util::Print("From UC_BasicCharacterAnimInstance::Montage_PlayInternal : " + IncomingMontageName + " has invalid PriorityTag!", FColor::Red, 10.f);
		return 0.f;
	}
	
	if (!m_MontagePriorityProjectSettings->GetPriority(CurPlayingPriorityTag, CurPlayingPriority))
	{
		const FString& CurPlayingMontageName = (*TargetGroupCurMontage)->GetName();
		UC_Util::Print("From UC_BasicCharacterAnimInstance::Montage_PlayInternal : " + CurPlayingMontageName + " has invalid PriorityTag!", FColor::Red, 10.f);
		return 0.f;
	}

	
	/* 구한 Priority 값 비교 */
	if (IncomingPriority >= CurPlayingPriority)
	{
		UC_Util::Print("Playing new Montage on " + TargetGroup.ToString(), FColor::Red, 10.f);
		
		m_CurPriorityAnimMontage[TargetGroup] = MontageToPlay;
		return Super::Montage_PlayInternal(MontageToPlay, BlendInSettings, InPlayRate, ReturnValueType, InTimeToStartMontageAt, bStopAllMontages);
	}
	
	// Priority가 현재 재생중인 Montage가 더 큰 경우, 새로이 재생하지 않고 그냥 return
	return 0.f;
}
