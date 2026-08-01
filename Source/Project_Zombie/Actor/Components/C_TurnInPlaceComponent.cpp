// Fill out your copyright notice in the Description page of Project Settings.


#include "C_TurnInPlaceComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "C_BasicPlayerAimComponent.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utility/C_Util.h"

UC_TurnInPlaceComponent::UC_TurnInPlaceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
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
//
//bool UC_TurnInPlaceComponent::StartTurnInPlaceMotion(float _YawRotDelta)
//{
//	// PoseState, HandState 및 Yaw Delta 값에 따른 Turn in place 몽타주 Animation 고르기
//	TMap<EHandState, FTurnInPlaceMontages>& PoseTargetMap	= m_OwnerPlayer->IsCrouching() ? m_CrouchTurnInPlaceMontages : m_StandTurnInPlaceMontages;
//	FTurnInPlaceMontages* TargetTurnInPlaceMontages			= PoseTargetMap.Find(m_OwnerPlayer->GetHandState());
//	
//	if (!TargetTurnInPlaceMontages)
//	{
//		UC_Util::Print("From TurinInPlaceComponent::StartTurnInPlaceMotion : Some HandState TurnInPlace montage is missing", FColor::Red, 10.f);
//		return false;
//	}
//	 
//	const TArray<UAnimMontage*>& TurnInPlaceMontagesToPlay = (_YawRotDelta > 90.f) ? TargetTurnInPlaceMontages->TurnRightMontages : TargetTurnInPlaceMontages->TurnLeftMontages;
//
//	if (TurnInPlaceMontagesToPlay.IsEmpty())
//	{
//		UC_Util::Print("From TurinInPlaceComponent::StartTurnInPlaceMotion : Some TurnInPlace montage is missing", FColor::Red, 10.f);
//		return false;
//	}
//	
//	// 이미 해당 Animation 을 재생중인 상황 (Addit group으로 체크함)
//	if (m_OwnerPlayer->GetMesh()->GetAnimInstance()->Montage_IsPlaying(TurnInPlaceMontagesToPlay[1])) return false;
//
//	// Default full body + Addit Lower body TurnInPlace 재생 처리
//	if (m_OwnerPlayer->GetAimComponent()->IsAiming())
//	{
//		m_OwnerPlayer->PlayAnimMontage(TurnInPlaceMontagesToPlay[1]);
//	}
//	else
//	{
//		for (UAnimMontage* Montage : TurnInPlaceMontagesToPlay)
//			m_OwnerPlayer->PlayAnimMontage(Montage);
//	}
//	
//	return true;
//}

void UC_TurnInPlaceComponent::CancelTurnInPlaceMotionIfNecessary()
{
	// Turn In Place중 움직이면 Turn In place 몽타주 끊고 해당 방향으로 바로 움직이게 하기

	TMap<EHandState, FTurnInPlaceMontages>& PoseTargetMap	= m_OwnerPlayer->IsCrouching() ? m_CrouchTurnInPlaceMontages : m_StandTurnInPlaceMontages;
	FTurnInPlaceMontages* TargetTurnInPlaceMontages			= PoseTargetMap.Find(m_OwnerPlayer->GetHandState());
	
	if (!TargetTurnInPlaceMontages) return;

	UAnimInstance* PlayerAnimInstance = m_OwnerPlayer->GetMesh()->GetAnimInstance();

	bool bStopped{};
	
	for (UAnimMontage* TurnRightMontage : TargetTurnInPlaceMontages->TurnRightMontages)
	{
		if (PlayerAnimInstance->Montage_IsPlaying(TurnRightMontage))
		{
			PlayerAnimInstance->Montage_Stop(0.2f, TurnRightMontage);
			bStopped = true;
		}
	}
	
	for (UAnimMontage* TurnLeftMontage : TargetTurnInPlaceMontages->TurnLeftMontages)
	{
		if (PlayerAnimInstance->Montage_IsPlaying(TurnLeftMontage))
		{
			PlayerAnimInstance->Montage_Stop(0.2f, TurnLeftMontage);
			bStopped = true;
		}
	}
	
	if (bStopped && m_OwnerPlayer->IsLocallyControlled())
		Server_RequestCancelTurnInPlaceMotion();
}

void UC_TurnInPlaceComponent::Server_RequestTurnInPlaceMotion_Implementation(bool _IsRight)
{
	// 해당 Player의 TurnInPlace 모션을 전역적으로 뿌려줌 (해당 당사자는 자신의 화면에서 이미 TurnInPlace 처리를 했음 -> 이미 TurnInPlace 중이라면 씹는 것으로 처리할 것)
	// PRINT_LOCAL(GetWorld(), "Server_RequestTurnInPlaceMotion", FColor::Red, 10.f);
	Multicast_StartTurnInPlaceMotion(_IsRight);
}

bool UC_TurnInPlaceComponent::Server_RequestTurnInPlaceMotion_Validate(bool _IsRight)
{
	return true;
}

void UC_TurnInPlaceComponent::Multicast_StartTurnInPlaceMotion_Implementation(bool _IsRight)
{
	// PRINT_LOCAL(GetWorld(), "Multicast_StartTurnInPlaceMotion", FColor::Cyan, 10.f);
	// TODO : 다른 ActorComponent나 RPC 호출을 받은 구현부에서, BeginPlay 이전에 받을 수 있는 상황임을 따져야 함
	// (특히 m_OwnerPlayer 초기화는 BeginPlay 시점이 가장 확실 -> 초기화 이전에 이미 Multicast 호출을 받는 경우가 종종 있었음)
	if (!m_OwnerPlayer) return; // 아직 초기화 이전단계
	
	if (m_OwnerPlayer->IsLocallyControlled()) return; // 자기자신이 플레이 중인 Player는 요청을 보내기 전에 선으로 이미 해당 모션을 취한 상황
	StartTurnInPlaceMotion(_IsRight, false); // Server에 해당 모션이 성공했을 때 재요청 x
}

bool UC_TurnInPlaceComponent::StartTurnInPlaceMotion(bool _IsRight, bool _RequestToServer)
{
	// PRINT_LOCAL(GetWorld(), "StartTurnInPlace", FColor::Red, 10.f);
	
	// PoseState, HandState 및 Yaw Delta 값에 따른 Turn in place 몽타주 Animation 고르기
	TMap<EHandState, FTurnInPlaceMontages>& PoseTargetMap	= m_OwnerPlayer->IsCrouching() ? m_CrouchTurnInPlaceMontages : m_StandTurnInPlaceMontages;
	FTurnInPlaceMontages* TargetTurnInPlaceMontages			= PoseTargetMap.Find(m_OwnerPlayer->GetHandState());
	
	if (!TargetTurnInPlaceMontages)
	{
		UC_Util::Print("From TurinInPlaceComponent::StartTurnInPlaceMotion : Some HandState TurnInPlace montage is missing", FColor::Red, 10.f);
		return false;
	}
	
	const TArray<UAnimMontage*>& TurnInPlaceMontagesToPlay = _IsRight ? TargetTurnInPlaceMontages->TurnRightMontages : TargetTurnInPlaceMontages->TurnLeftMontages;

	if (TurnInPlaceMontagesToPlay.IsEmpty())
	{
		UC_Util::Print("From TurinInPlaceComponent::StartTurnInPlaceMotion : Some TurnInPlace montage is missing", FColor::Red, 10.f);
		return false;
	}
	
	// 이미 해당 Animation 을 재생중인 상황 (Addit group으로 체크함)
	if (m_OwnerPlayer->GetMesh()->GetAnimInstance()->Montage_IsPlaying(TurnInPlaceMontagesToPlay[1])) return false;

	// Default full body + Addit Lower body TurnInPlace 재생 처리
	if (m_OwnerPlayer->GetAimComponent()->IsAiming() )
	{
		//UC_Util::Print("TurnInPlace Lower", FColor::MakeRandomColor(), 10.f);
		m_OwnerPlayer->PlayAnimMontage(TurnInPlaceMontagesToPlay[1]);
	}
	else
	{
		for (UAnimMontage* Montage : TurnInPlaceMontagesToPlay)
			m_OwnerPlayer->PlayAnimMontage(Montage);
	}

	// 자신의 Local Player -> TurnInPlace 처리 완료
	// TurnInPlace 일어났다고 나머지 사람들에게 뿌리기
	if (_RequestToServer) Server_RequestTurnInPlaceMotion(_IsRight);
	
	return true;
}

void UC_TurnInPlaceComponent::Multicast_CancelTurnInPlaceMotion_Implementation()
{
	if (!m_OwnerPlayer) return;
	if (m_OwnerPlayer->IsLocallyControlled()) return;
	CancelTurnInPlaceMotionIfNecessary();
}

void UC_TurnInPlaceComponent::Server_RequestCancelTurnInPlaceMotion_Implementation()
{
	Multicast_CancelTurnInPlaceMotion();
}

bool UC_TurnInPlaceComponent::Server_RequestCancelTurnInPlaceMotion_Validate()
{
	return true;
}

void UC_TurnInPlaceComponent::InitTurnInPlaceMontages()
{
	m_StandTurnInPlaceMontages.Empty();
    m_CrouchTurnInPlaceMontages.Empty();

    struct FMontagePathConfig
    {
	    FString Suffix{};
	    bool	bIsLeft{};
    };

    static const FMontagePathConfig PathConfigs[] = 
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
