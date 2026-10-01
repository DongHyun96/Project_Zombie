// Fill out your copyright notice in the Description page of Project Settings.


#include "C_UtilActor.h"


AC_UtilActor::AC_UtilActor()
{
#if WITH_EDITOR
	PrimaryActorTick.bCanEverTick = true;
#else
	PrimaryActorTick.bCanEverTick = false;
#endif
	
}

void AC_UtilActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AC_UtilActor::Tick(float DeltaTime)
{
	// Editing 환경에서만 Tick 돔
	
	Super::Tick(DeltaTime);
	m_CurTickColor = FColor::MakeRandomColor();
}

