// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerProfileComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_PingSystemComponent.h"
#include "GameModeAndManager/C_UIManager.h"
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
	
	// TODO : 이 Test 라인 지울 것 (일단 서버에서 일괄 랜덤 적용한 색상으로 적용)
	if (m_OwnerPlayer->HasAuthority()) m_PlayerSelectedColor = FColor::MakeRandomColor();
	
	if (!m_OwnerPlayer->IsLocallyControlled())
		UI_MANAGER(GetWorld())->GetMainHUDWidget()->GetOtherPlayerStatWidget()->RegisterOtherPlayer(m_OwnerPlayer);
}

void UC_PlayerProfileComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UC_PlayerProfileComponent, m_PlayerSelectedColor);
	DOREPLIFETIME(UC_PlayerProfileComponent, m_PlayerName);
}

void UC_PlayerProfileComponent::OnRep_PlayerName()
{
	if (!m_OwnerPlayer) return;
	
	if (!m_OwnerPlayer->IsLocallyControlled())
		UI_MANAGER(GetWorld())->GetMainHUDWidget()->GetOtherPlayerStatWidget()->RegisterOtherPlayer(m_OwnerPlayer);	
}

void UC_PlayerProfileComponent::OnRep_PlayerSelectedColor()
{
	if (!m_OwnerPlayer) return;
	
	if (!m_OwnerPlayer->IsLocallyControlled())
		UI_MANAGER(GetWorld())->GetMainHUDWidget()->GetOtherPlayerStatWidget()->RegisterOtherPlayer(m_OwnerPlayer);
}

