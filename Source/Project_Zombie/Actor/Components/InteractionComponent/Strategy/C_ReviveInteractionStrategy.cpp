// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ReviveInteractionStrategy.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "../C_InteractionComponent.h"

#include "Utility/C_Util.h"

UC_ReviveInteractionStrategy::UC_ReviveInteractionStrategy()
{
	m_InteractionDuration = 3.0f;
}

bool UC_ReviveInteractionStrategy::CanStartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor) const
{
	AC_BasicPlayer* TargetPlayer = Cast<AC_BasicPlayer>(_TargetActor);

	if (!_Interactor || !TargetPlayer)
		return false;

	if (_Interactor->IsDead())
		return false;

	// 다운되어 있어야지 상호작용 가능
	return TargetPlayer->IsDead();
}

bool UC_ReviveInteractionStrategy::StartInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	AC_BasicPlayer* TargetPlayer = Cast<AC_BasicPlayer>(_TargetActor);

	if (!CanStartInteraction(_Interactor, TargetPlayer))
		return false;
	
	UC_Util::Print("----------->StartReviveInteraction", FColor::Red, 10.f);

	_Interactor->SetPlayerStateOnServer(EPlayerState::Reviving);

	return true;
}

void UC_ReviveInteractionStrategy::CancleInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	if (!_Interactor || !_TargetActor)
		return;

	_Interactor->SetPlayerStateOnServer(EPlayerState::Idle);
}

void UC_ReviveInteractionStrategy::CompleteInteraction(AC_BasicPlayer* _Interactor, AActor* _TargetActor)
{
	if (!_Interactor || !_TargetActor)
		return;

	AC_BasicPlayer* TargetPlayer = Cast<AC_BasicPlayer>(_TargetActor);
	if (!TargetPlayer)
		return;

	// 서버에서만 구조 처리
	if (!TargetPlayer->HasAuthority())
		return;

	if (!TargetPlayer->IsDead())
		return;

	_Interactor->SetPlayerStateOnServer(EPlayerState::Idle);

	// 구조 대상 일으키기 시작
	TargetPlayer->StartGettingUp();
}
