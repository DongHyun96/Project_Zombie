// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerWorldPingActor.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "UI/Ping/C_PingWidget.h"
#include "Utility/C_Util.h"


AC_PlayerWorldPingActor::AC_PlayerWorldPingActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AC_PlayerWorldPingActor::BeginPlay()
{
	Super::BeginPlay();
	
	// SpawnActor Param을 BasicPlayer로 지정해줌
	m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
	if (!m_OwnerPlayer)
		UC_Util::Print("[AC_PlayerWorldPingActor::BeginPlay] : OwnerPlayer casting failed", FColor::Red, 10.f);

	if (m_PingWidget) m_PingWidget->SetOwnerPlayer(m_OwnerPlayer);
}

void AC_PlayerWorldPingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

