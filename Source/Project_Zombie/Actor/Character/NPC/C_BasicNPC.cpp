// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Character/NPC/C_BasicNPC.h"

AC_BasicNPC::AC_BasicNPC()
{
}

void AC_BasicNPC::BeginPlay()
{
	Super::BeginPlay();
}

ETeamAttitude::Type AC_BasicNPC::GetTeamAttitudeTowards(const AActor& _Other) const
{
	const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(&_Other);

	if (TeamAgent)
	{
		FGenericTeamId OtherID = TeamAgent->GetGenericTeamId();

		if (GetGenericTeamId() == OtherID)
		{
			return ETeamAttitude::Friendly;
		}
		else
		{
			return ETeamAttitude::Hostile;
		}
	}

	// IGenericTeamAgentInterface 기능이 없는 Actor 인 경우 중립
	return ETeamAttitude::Neutral;
}
