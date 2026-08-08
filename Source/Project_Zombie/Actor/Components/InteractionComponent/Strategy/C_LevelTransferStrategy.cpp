// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Components/InteractionComponent/Strategy/C_LevelTransferStrategy.h"

#include "Actor/Character/Player/C_BasicPlayer.h"

bool UC_LevelTransferStrategy::CanStartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const
{
	//return Super::CanStartInteraction(_Interactor, _TargetActor);
	return true;
}

bool UC_LevelTransferStrategy::StartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	return true;
	//return Super::StartInteraction(_Interactor, _TargetActor);
}

void UC_LevelTransferStrategy::CancleInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	Super::CancleInteraction(_Interactor, _TargetActor);
}

void UC_LevelTransferStrategy::CompleteInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	//Super::CompleteInteraction(_Interactor, _TargetActor);

	if (!_Interactor) return;

	// 컴포넌트 설계상 EInteractionNetType::Server로 동작하므로
	// 이 함수가 실행되는 시점은 이미 '서버'입니다. (안전하게 HasAuthority 체크 추가)
	if (_Interactor->HasAuthority())
	{
		UWorld* World = GetWorld();
		if (World && !TargetLevelName.IsEmpty())
		{
			// 리슨 서버 환경에서 모든 유저(호스트+클라이언트)를 함께 데려가려면 ServerTravel을 사용합니다.
			FString TravelURL = TargetLevelName;

			// 다음 레벨도 멀티플레이 세션(리슨 서버 상태)이 유지되도록 ?Listen 옵션을 붙여줍니다.
			if (!TravelURL.Contains(TEXT("?Listen")))
			{
				TravelURL += TEXT("?Listen");
			}

			UE_LOG(LogTemp, Log, TEXT("Match Start! Traveling to %s"), *TravelURL);
            
			// 전원 이동 감행
			World->ServerTravel(TravelURL);
		}
	}
}
