// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GameMainHUD.h"

#include "InformWidget/C_InformWidget.h"
#include "PlayerStatHUD/C_PlayerStatWidget.h"
#include "InteractionWidget/C_InteractionWidget.h"

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

bool UC_GameMainHUD::UpdateBoostBar(float _Boost, float _MaxBoost)
{
	return PlayerStatWidget->UpdateBoostBar(_Boost, _MaxBoost);
}

void UC_GameMainHUD::ChangeBoostBarColor(bool IsExhausted)
{
	PlayerStatWidget->ToggleBoostBarColor(IsExhausted);
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

bool UC_GameMainHUD::UpdateFireMode(EFireMode _NewFireMode)
{
	return PlayerStatWidget->UpdateFireMode(_NewFireMode);
}

void UC_GameMainHUD::ActivateInteractionUI(const FText& _InteractionText)
{
	if (!InteractionWidget)
		return;

	InteractionWidget->ActivateInteraction(_InteractionText);
}

void UC_GameMainHUD::DeactivateInteractionUI()
{
	if (!InteractionWidget)
		return;

	InteractionWidget->DeactivateInteraction();
}

void UC_GameMainHUD::ActivateInteractionTimer(float _Duration)
{
	if (!InteractionWidget)
		return;

	InteractionWidget->ActivateInteractionTimer(_Duration);
}

void UC_GameMainHUD::DeactivateInteractionTimer()
{
	if (!InteractionWidget)
		return;

	InteractionWidget->DeactivateInteractionTimer();
}

void UC_GameMainHUD::ToggleFriendListWidget(bool _Visible)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	const ESlateVisibility PrevVisibility = FriendListWidget->GetVisibility();
	
	if (_Visible)
	{
		// InputMode 수정도 들어가 있어서 중복 처리로 인한 꼬임 방지를 위해 기존의 상태와 동일한 Input을 받았다면, early return 처리함
		if (PrevVisibility == ESlateVisibility::Visible) return;
		
		FriendListWidget->SetVisibility(ESlateVisibility::Visible);
		
		FInputModeGameAndUI InputMode{};
		InputMode.SetWidgetToFocus(nullptr);

		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
		
		return;
	}

	// InputMode 수정도 들어가 있어서 중복 처리로 인한 꼬임 방지를 위해 기존의 상태와 동일한 Input을 받았다면, early return 처리함
	if (PrevVisibility == ESlateVisibility::Collapsed || PrevVisibility == ESlateVisibility::Hidden) return;
	
	FriendListWidget->SetVisibility(ESlateVisibility::Collapsed);
		
	FInputModeGameOnly InputMode{};
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(false);
}

bool UC_GameMainHUD::AddPlayerWarningLog(const FString& WarningLog, const FColor& _LogColor)
{
	return InformWidget->AddPlayerWarningLog(WarningLog, _LogColor);
}
