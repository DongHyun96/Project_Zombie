// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerProfileComponent.h"

#include "OnlineSubsystem.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "GameFramework/PlayerState.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Net/UnrealNetwork.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "UI/MainHUD/PlayerStatHUD/C_OtherPlayerStatWidget.h"
#include "Utility/C_Util.h"


UC_PlayerProfileComponent::UC_PlayerProfileComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;
	
	SetIsReplicatedByDefault(true);
}


void UC_PlayerProfileComponent::BeginPlay()
{
	Super::BeginPlay();

	m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
	if (!m_OwnerPlayer)
	{
		UC_Util::Print("From UC_PlayerProfileComponent::BeginPlay : OwnerPlayer casting failed!", FColor::Red, 10.f);
		return;
	}

	// 서버 환경에서 랜덤한 Color를 부여하고, 만약 이 캐릭터가 내 플레이어가 아니라면, 서버의 경우 바로 등록처리를 한다
	if (m_OwnerPlayer->HasAuthority())
	{
		m_PlayerSelectedColor = FColor::MakeRandomColor();
		OnRep_PlayerSelectedColor();
	}
}

void UC_PlayerProfileComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UC_PlayerProfileComponent, m_PlayerSelectedColor);
}

void UC_PlayerProfileComponent::OnRep_PlayerSelectedColor()
{
	// 아직 이 Component의 BeginPlay가 불리지 않은 시점
	if (!m_OwnerPlayer) m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
	if (!m_OwnerPlayer)
	{
		// 그래도 OwnerPlayer가 제대로 초기화 처리가 되지 않은 상황
		UC_Util::Print("[UC_PlayerProfileComponent::OnRep_PlayerSelectedColor] : OwnerPlayer casting failed!", FColor::Red, 10.f);
		return;
	}
	
	if (!m_OwnerPlayer->IsLocallyControlled())
	{
		FTimerDelegate RegDelegate = FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			// 아직 이름을 구할 수 없을 경우 기다림
			if (!m_OwnerPlayer->GetPlayerState()) return;
			
			UC_GameMainHUD* MainHUD = MAIN_HUD(GetWorld());
			UC_OtherPlayerStatWidget* OtherStatWidget = MainHUD ? MainHUD->GetOtherPlayerStatWidget() : nullptr;
			if (!OtherStatWidget) return;
			

			// UI가 제대로 초기화되었을 때 등록하고 타이머 종료
			OtherStatWidget->RegisterOtherPlayer(m_OwnerPlayer);
			GetWorld()->GetTimerManager().ClearTimer(m_OtherPlayerRegTimerHandle);
		});

		GetWorld()->GetTimerManager().SetTimer(m_OtherPlayerRegTimerHandle, RegDelegate, 0.1f, true);
	}
}

