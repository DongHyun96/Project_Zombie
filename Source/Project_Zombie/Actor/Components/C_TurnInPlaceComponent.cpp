// Fill out your copyright notice in the Description page of Project Settings.


#include "C_TurnInPlaceComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utility/C_Util.h"

UC_TurnInPlaceComponent::UC_TurnInPlaceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UC_TurnInPlaceComponent::BeginPlay()
{
	Super::BeginPlay();
	
	m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
	if (!m_OwnerPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("From UC_TurnInPlaceComponent::BeginPlay : OwnerPlayer init failed!"));
		UC_Util::Print("From UC_TurnInPlaceComponent::BeginPlay : OwnerPlayer init failed!");
	}
	
	InitTurnInPlaceMontages();
}

bool UC_TurnInPlaceComponent::StartTurnInPlaceMotion(float _YawRotDelta)
{
	// PoseState, HandState 및 Yaw Delta 값에 따른 Turn in place 몽타주 Animation 고르기
	TMap<EHandState, FTurnInPlaceMontages>& PoseTargetMap	= m_OwnerPlayer->IsCrouching() ? m_CrouchTurnInPlaceMontages : m_StandTurnInPlaceMontages;
	FTurnInPlaceMontages* TargetTurnInPlaceMontages			= PoseTargetMap.Find(m_OwnerPlayer->GetHandState());
	
	if (!TargetTurnInPlaceMontages)
	{
		UC_Util::Print("From TurinInPlaceComponent::StartTurnInPlaceMotion : Some HandState TurnInPlace montage is missing", FColor::Red, 10.f);
		return false;
	}
	 
	const TArray<UAnimMontage*>& TurnInPlaceMontagesToPlay = (_YawRotDelta > 90.f) ? TargetTurnInPlaceMontages->TurnRightMontages : TargetTurnInPlaceMontages->TurnLeftMontages;

	if (TurnInPlaceMontagesToPlay.IsEmpty())
	{
		UC_Util::Print("From TurinInPlaceComponent::StartTurnInPlaceMotion : Some TurnInPlace montage is missing", FColor::Red, 10.f);
		return false;
	}
	
	// 이미 해당 Animation 을 재생중인 상황 (Addit group으로 체크함)
	if (m_OwnerPlayer->GetMesh()->GetAnimInstance()->Montage_IsPlaying(TurnInPlaceMontagesToPlay[1])) return false;

	// Default full body + Addit Lower body TurnInPlace 재생 처리
	for (UAnimMontage* Montage : TurnInPlaceMontagesToPlay)
		m_OwnerPlayer->PlayAnimMontage(Montage);
	
	return true;
}

void UC_TurnInPlaceComponent::CancelTurnInPlaceMotionIfNecessary()
{
	// Turn In Place중 움직이면 Turn In place 몽타주 끊고 해당 방향으로 바로 움직이게 하기

	TMap<EHandState, FTurnInPlaceMontages>& PoseTargetMap	= m_OwnerPlayer->IsCrouching() ? m_CrouchTurnInPlaceMontages : m_StandTurnInPlaceMontages;
	FTurnInPlaceMontages* TargetTurnInPlaceMontages			= PoseTargetMap.Find(m_OwnerPlayer->GetHandState());
	
	if (!TargetTurnInPlaceMontages) return;

	UAnimInstance* PlayerAnimInstance = m_OwnerPlayer->GetMesh()->GetAnimInstance();
	
	for (UAnimMontage* TurnRightMontage : TargetTurnInPlaceMontages->TurnRightMontages)
	{
		if (PlayerAnimInstance->Montage_IsPlaying(TurnRightMontage))
			PlayerAnimInstance->Montage_Stop(0.2f, TurnRightMontage);
	}
	
	for (UAnimMontage* TurnLeftMontage : TargetTurnInPlaceMontages->TurnLeftMontages)
	{
		if (PlayerAnimInstance->Montage_IsPlaying(TurnLeftMontage))
			PlayerAnimInstance->Montage_Stop(0.2f, TurnLeftMontage);
	}
}

void UC_TurnInPlaceComponent::InitTurnInPlaceMontages()
{
	m_StandTurnInPlaceMontages.Empty();
    m_CrouchTurnInPlaceMontages.Empty();

    struct FMontagePathConfig
    {
	    FString Suffix{};
	    bool bIsLeft{};
    };

    FMontagePathConfig PathConfigs[] = 
    {
        { TEXT("TurnLeft_Montage"),       true },
        { TEXT("TurnLeft_Montage_Lower"), true },
        { TEXT("TurnRight_Montage"),      false },
        { TEXT("TurnRight_Montage_Lower"),false }
    };

    for (uint8 HandIdx = 0; HandIdx < static_cast<uint8>(EHandState::Max); ++HandIdx)
    {
        EHandState CurrentHandState = static_cast<EHandState>(HandIdx);
        
        for (uint8 PoseIdx = 0; PoseIdx <= 1; ++PoseIdx)
        {
            if (PoseIdx == 0) m_StandTurnInPlaceMontages.FindOrAdd(CurrentHandState);
            else			  m_CrouchTurnInPlaceMontages.FindOrAdd(CurrentHandState);

            TMap<EHandState, FTurnInPlaceMontages>& TargetPoseMap = (PoseIdx == 0) ? m_StandTurnInPlaceMontages : m_CrouchTurnInPlaceMontages;
	                                                                    
            for (const auto& Config : PathConfigs)
            {
                const FString MontagePath = FString::Printf
            	(
                    TEXT("/Game/DongHyun/Anim/PlayerAnims/TurnInPlace/%u%u_%s"),
                    HandIdx,
                    PoseIdx,
                    *Config.Suffix
                );

                if (UAnimMontage* LoadedMontage = LoadObject<UAnimMontage>(nullptr, *MontagePath))
                {
                    if (Config.bIsLeft) TargetPoseMap[CurrentHandState].TurnLeftMontages.Add(LoadedMontage);
                    else			    TargetPoseMap[CurrentHandState].TurnRightMontages.Add(LoadedMontage);
                	
                }
                else UC_Util::Print("UC_TurnInPlaceComponent::InitTurnInPlaceMontages : AnimMontage Loaded failed", FColor::Red, 10.f);
            }
        }
    }
}
