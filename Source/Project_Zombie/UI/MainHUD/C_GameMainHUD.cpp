// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GameMainHUD.h"

#include "PlayerStatHUD/C_PlayerStatWidget.h"

void UC_GameMainHUD::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UC_GameMainHUD::NativeConstruct()
{
	Super::NativeConstruct();
}

void UC_GameMainHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

bool UC_GameMainHUD::UpdateHPBar(float _HP, float _MaxHP)
{
	return PlayerStatWidget->UpdateHPBar(_HP, _MaxHP);
}

bool UC_GameMainHUD::UpdateBoostBar(float _Boost, float _MaxBoost)
{
	return PlayerStatWidget->UpdateBoostBar(_Boost, _MaxBoost);
}
