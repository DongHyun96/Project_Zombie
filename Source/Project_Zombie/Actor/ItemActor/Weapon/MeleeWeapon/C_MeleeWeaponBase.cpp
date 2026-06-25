// Fill out your copyright notice in the Description page of Project Settings.


#include "C_MeleeWeaponBase.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "GameMode/C_UIManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"


AC_MeleeWeaponBase::AC_MeleeWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AC_MeleeWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AC_MeleeWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_MeleeWeaponBase::StartAttack()
{
	Super::StartAttack();
}

void AC_MeleeWeaponBase::StopAttack()
{
	Super::StopAttack();
}

void AC_MeleeWeaponBase::Reload()
{
	Super::Reload();
}

bool AC_MeleeWeaponBase::AttachToHolster(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		m_HolsterSocketName
	);
	
	return bIsAttached;
}

bool AC_MeleeWeaponBase::AttachToHand(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false

	// Main HUD MeleeWeapon 종류로 초기화
	// TODO : 각 MeleeWeapon에 맞는 이미지 아이콘(?) 표시해주면 좋을 듯 (일단은 AmmoInfo쪽 정보 감추는 처리로 함)
	if (AC_UIManager* UIManager = Cast<AC_UIManager>(Player->GetController<APlayerController>()->GetHUD()))
		UIManager->GetMainHUDWidget()->ToggleAmmoInfoVisibility(false);

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		m_HandSocketName
	);
	
	if (bIsAttached)
    		Player->SetHandState(EHandState::WeaponMelee);
	
	return bIsAttached;
}

