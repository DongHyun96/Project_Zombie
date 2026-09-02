// Fill out your copyright notice in the Description page of Project Settings.


#include "C_SkinWidget.h"

#include "Components/Button.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/InteractionComponent/C_InteractionComponent.h"

void UC_SkinWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Origin)
		Button_Origin->OnClicked.AddUniqueDynamic(
			this,
			&UC_SkinWidget::OnClickOrigin);

	if (Button_Purple)
		Button_Purple->OnClicked.AddUniqueDynamic(
			this,
			&UC_SkinWidget::OnClickPurple);

	if (Button_Red)
		Button_Red->OnClicked.AddUniqueDynamic(
			this,
			&UC_SkinWidget::OnClickRed);

	if (Button_Green)
		Button_Green->OnClicked.AddUniqueDynamic(
			this,
			&UC_SkinWidget::OnClickGreen);

	if (Button_Blue)
		Button_Blue->OnClicked.AddUniqueDynamic(
			this,
			&UC_SkinWidget::OnClickBlue);

	if (Button_Close)
		Button_Close->OnClicked.AddUniqueDynamic(
			this,
			&UC_SkinWidget::OnClickClose);
}

void UC_SkinWidget::SetUsePlayer(AC_BasicPlayer* InPlayer)
{
	m_UsePlayer = InPlayer;
}

void UC_SkinWidget::SelectSkin(EPlayerSkin InSkin)
{
	if (!m_UsePlayer)
		return;

	m_UsePlayer->Server_RequestApplySkin(InSkin);
}

void UC_SkinWidget::OnClickOrigin()
{
	SelectSkin(EPlayerSkin::Origin);
}

void UC_SkinWidget::OnClickPurple()
{
	SelectSkin(EPlayerSkin::Purple);
}

void UC_SkinWidget::OnClickRed()
{
	SelectSkin(EPlayerSkin::Red);
}

void UC_SkinWidget::OnClickGreen()
{
	SelectSkin(EPlayerSkin::Green);
}

void UC_SkinWidget::OnClickBlue()
{
	SelectSkin(EPlayerSkin::Blue);
}

void UC_SkinWidget::OnClickClose()
{
	if (!m_UsePlayer)
		return;

	UC_InteractionComponent* InteractionComp = m_UsePlayer->GetInteractionComponent();
	if (!InteractionComp)
		return;

	InteractionComp->CancleInteract();
}
