// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PlayerState.h"

#include "Net/UnrealNetwork.h"

AC_PlayerState::AC_PlayerState()
{
}

void AC_PlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AC_PlayerState, m_bIsHost);
}
