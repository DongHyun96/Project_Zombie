#include "Actor/Components/C_EquippedComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "GameMode/C_UIManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Utility/C_Util.h"

UC_EquippedComponent::UC_EquippedComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UC_EquippedComponent::BeginPlay()
{
	Super::BeginPlay();

	
	m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
	if (!m_OwnerPlayer)
	{
		UC_Util::Print("From UC_EquippedComponent::BeginPlay : OwnerPlayer init failed!", FColor::Red, 10.f);
		UE_LOG(LogTemp, Error, TEXT("From UC_EquippedComponent::BeginPlay : OwnerPlayer init failed!"));
	}

	// 각 Slot칸 nullptr로 미리 확보
	m_Weapons.SetNum(static_cast<uint8>(EWeaponSlot::Max));
}

AC_WeaponBase* UC_EquippedComponent::SetSlotWeapon(EWeaponSlot TargetSlot, AC_WeaponBase* WeaponToEquip)
{
	const uint8 TargetSlotIdx = static_cast<uint8>(TargetSlot); 
	
	AC_WeaponBase* PrevSlotWeapon = m_Weapons[TargetSlotIdx];

    // 들어온 슬롯의 이전 무기가 존재할 때, 이전 무기 해제 
    // (이 부분 배그에서는 DetachFromActor 처리로 바닥에 해당 무기 내려놓게 처리를 해주었던 것 같음)
	// 기존에 들고 있었던 무기를 어떤식으로 처리할지는 아직 미정 -> 일단 특별한 처리는 해주지 않음
    if (PrevSlotWeapon)
    {
    	// ...
    }

    m_Weapons[TargetSlotIdx] = WeaponToEquip; // 새로 들어온 무기로 교체
    
    if (!m_Weapons[TargetSlotIdx]) // Slot에 새로 지정한 무기가 nullptr -> early return
    {
        if (CurWeaponType == TargetSlot) // 현재 손에 들고 있는 무기를 Slot에서 강제로 뺀 상황
        {
            NextWeaponType  = EWeaponSlot::None;
            CurWeaponType   = EWeaponSlot::None;
        	m_OwnerPlayer->SetHandState(EHandState::UnArmed);

        	// Ammo info 정보 MainHUD에서 숨기기 -> 해당 인원이 자기자신인지 확인을 해주어야할..듯?
        	if (AC_UIManager* UIManager = Cast<AC_UIManager>(m_OwnerPlayer->GetController<APlayerController>()->GetHUD()))
        		UIManager->GetMainHUDWidget()->ToggleAmmoInfoVisibility(false);
        }

        return PrevSlotWeapon;
    }

	// Throwable의 경우 장착된 모습 보이지 않게끔 처리
    if (TargetSlot == EWeaponSlot::ThrowableWeapon)
        m_Weapons[TargetSlotIdx]->SetActorHiddenInGame(true);

    // m_Weapons[InSlot]->SetRelativeTranformToInitial(); // TODO : 무기 부착 시 위치 이상해지면, 이 함수처리 고려할 것
    m_Weapons[TargetSlotIdx]->AttachToHolster(m_OwnerPlayer->GetMesh());
    
    return PrevSlotWeapon;
}

bool UC_EquippedComponent::ChangeCurWeapon(EWeaponSlot _ChangeTo)
{
	const uint8 ChangeToIdx = static_cast<uint8>(_ChangeTo);

	
	
	return false;
}
