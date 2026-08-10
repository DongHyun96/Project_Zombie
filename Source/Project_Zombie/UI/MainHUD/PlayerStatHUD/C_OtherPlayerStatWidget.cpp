// Fill out your copyright notice in the Description page of Project Settings.


#include "C_OtherPlayerStatWidget.h"

#include "C_MiniHPBarWidget.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/PlayerProfileComponent/C_PlayerProfileComponent.h"
#include "Utility/C_Util.h"

void UC_OtherPlayerStatWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	m_MiniHPs.Add(MiniHP_3);
	m_MiniHPs.Add(MiniHP_2);	
	m_MiniHPs.Add(MiniHP_1);	
	m_MiniHPs.Add(MiniHP_0);	
}

void UC_OtherPlayerStatWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UC_OtherPlayerStatWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UC_OtherPlayerStatWidget::RegisterOtherPlayer(AC_BasicPlayer* _Player)
{
	if (!_Player)
	{
		UC_Util::Print("[UC_OtherPlayerStatWidget::RegisterOtherPlayer] : Received nullptr player", FColor::Red, 10.f);
		return;
	}
	
	if (m_MiniHPs.IsEmpty())
	{
		UC_Util::Print("[UC_OtherPlayerStatWidget::RegisterOtherPlayer] : NativeOnInitialized not called or not enough MiniHP count!", FColor::Red, 10.f);
		return;
	}

	if (UC_MiniHPBarWidget** TargetWidget = m_RegisteredWidget.Find(_Player))
	{
		(*TargetWidget)->Activate(_Player);
		return;
	}
	
	// 신규 등록
	UC_MiniHPBarWidget* MiniHPBarWidget = m_MiniHPs.Pop();
	MiniHPBarWidget->Activate(_Player);
	m_RegisteredWidget.Add(_Player, MiniHPBarWidget);
	
	_Player->ToggleNameTagVisible(true);
	_Player->SetNameTagWidgetInfo(_Player->GetPlayerProfileComponent()->GetPlayerName(), _Player->GetPlayerProfileComponent()->GetPlayerSelectedColor());
}

void UC_OtherPlayerStatWidget::UpdateHPBar(AC_BasicPlayer* _TargetPlayer, float _HPRatio)
{
	if (!_TargetPlayer) return;
	
	if (UC_MiniHPBarWidget** TargetWidget = m_RegisteredWidget.Find(_TargetPlayer))
	{
		(*TargetWidget)->UpdateHPBar(_HPRatio);
		return;
	}
	
	UC_Util::Print("[UC_OtherPlayerStatWidget::UpdateHPBar] : No TargetWidget found", FColor::Red, 10.f);
}
