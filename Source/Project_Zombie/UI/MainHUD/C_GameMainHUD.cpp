// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GameMainHUD.h"

#include "InformWidget/C_InformWidget.h"
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

bool UC_GameMainHUD::UpdateHPBarRatio(float _Ratio)
{
	return PlayerStatWidget->UpdateHPBar(_Ratio);
}

bool UC_GameMainHUD::UpdateBoostBar(float _Boost, float _MaxBoost)
{
	return PlayerStatWidget->UpdateBoostBar(_Boost, _MaxBoost);
}

bool UC_GameMainHUD::ToggleAmmoInfoVisibility(bool _Visible, EFireMode _FireMode, int32 _MagazineAmmo, int32 _LeftAmmoTotalCount)
{
	return PlayerStatWidget->ToggleAmmoInfoVisibility(_Visible, _FireMode, _MagazineAmmo, _LeftAmmoTotalCount);
}

void UC_GameMainHUD::UpdateMagazineAmmoCount(int32 _AmmoCount)
{
	PlayerStatWidget->UpdateMagazineAmmoCount(_AmmoCount);
}

void UC_GameMainHUD::UpdateLeftAmmoTotalCount(int32 _LeftAmmoTotalCount)
{
	PlayerStatWidget->UpdateLeftAmmoTotalCount(_LeftAmmoTotalCount);
}

bool UC_GameMainHUD::AddPlayerWarningLog(const FString& WarningLog)
{
	return InformWidget->AddPlayerWarningLog(WarningLog);
}
