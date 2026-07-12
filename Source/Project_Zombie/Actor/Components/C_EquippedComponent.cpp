#include "Actor/Components/C_EquippedComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "GameModeAndManager/C_UIManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Utility/C_Util.h"

UC_EquippedComponent::UC_EquippedComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 각 Slot칸 nullptr로 미리 확보
	m_Weapons.SetNum(static_cast<uint8>(EWeaponSlot::Max));
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
	
	// Test용으로 무기 미리 스폰
	for (const TTuple<EWeaponSlot, TSubclassOf<AC_WeaponBase>>& WeaponClassPair : m_WeaponClassToSpawn)
	{
		AC_WeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<AC_WeaponBase>(WeaponClassPair.Value);
		
		if (IsValid(SpawnedWeapon))
			SetSlotWeapon(WeaponClassPair.Key, SpawnedWeapon);
	}
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
        if (m_CurWeaponTypeIdx == TargetSlotIdx) // 현재 손에 들고 있는 무기를 Slot에서 강제로 뺀 상황
        {
            m_NextWeaponTypeIdx  = static_cast<uint8>(EWeaponSlot::None);
            m_CurWeaponTypeIdx   = static_cast<uint8>(EWeaponSlot::None);
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

    // m_Weapons[InSlot]->SetRelativeTransformToInitial(); // TODO : 무기 부착 시 위치 이상해지면, 이 함수처리 고려할 것
    m_Weapons[TargetSlotIdx]->AttachToHolster(m_OwnerPlayer->GetMesh());
    
    return PrevSlotWeapon;
}

bool UC_EquippedComponent::ChangeCurWeapon(EWeaponSlot _ChangeTo)
{
	const uint8 ChangeToIdx = static_cast<uint8>(_ChangeTo);
	const uint8 NoneSlotIdx = static_cast<uint8>(EWeaponSlot::None);

	// 현재 들고 있는 무기가 Valid하고, 해당 무기의 교체 모션이 이미 진행중이라면
	// 새로 들어온 교체 요청 처리 x
	// m_bIsCurrentlyChangingWeapon으로 확인하지 않은 이유 -> 글쎄다?
	// TODO : 만약 문제 생긴다면 이 처리로 아래 if 문 처리 대체할 것
	/*if (m_Weapons[m_CurWeaponTypeIdx])
	{
		UAnimMontage* CurWeaponDrawMontage   = m_Weapons[m_CurWeaponTypeIdx]->GetDrawMontage();
		UAnimMontage* CurWeaponSheathMontage = m_Weapons[m_CurWeaponTypeIdx]->GetSheathMontage();
		UAnimInstance* OwnerAnimInstance     = m_OwnerPlayer->GetMesh()->GetAnimInstance(); 		

		if (OwnerAnimInstance->Montage_IsPlaying(CurWeaponDrawMontage) ||
			OwnerAnimInstance->Montage_IsPlaying(CurWeaponSheathMontage))
			return false;
	}*/

	// 현재 무기교체처리가 이미 진행되고 있는 경우
	if (m_bIsCurrentlyChangingWeapon) return false;
	
	
	// 방어 예외처리 코드에 안정성을 위해 NextWeapon을 None으로 초기화 처리 모두 해둠
	// 위의 이미 교체처리가 이루어지는 상황에서의 방어코드는 NextWeapon 종류를 바꾸면 안됨 (바꾸는 와중이라...)
	
	// 현재 무기와 다음으로 교체할 무기 종류가 같으면 Swap할 필요 없음
	if (m_CurWeaponTypeIdx == ChangeToIdx)
	{
		m_NextWeaponTypeIdx = NoneSlotIdx;
		return false;
	}
	
	// NextWeaponType이 None이 아니고, 바꾸려는 무기 슬롯에 무기가 없을 때, Swap 처리 불가
	if (_ChangeTo != EWeaponSlot::None && !m_Weapons[ChangeToIdx])
	{
		m_NextWeaponTypeIdx = NoneSlotIdx;
		return false;
	}
	
	/* 무기 Swapping 처리 시작 */
	m_NextWeaponTypeIdx = ChangeToIdx;
	
	/* 현재 무기를 착용중이지 않은 UnArmed 상태일 때, 또는 현재 슬롯에 장착된 무기가 없을 때 */ 
	// 다음 무기 Draw만 재생
	if (m_CurWeaponTypeIdx == NoneSlotIdx || !GetCurWeapon())
	{
		// 만약 다음에 바꿀 무기가 None이거나 다음에 바꿀 무기 슬롯에 무기가 없을 때
		if (_ChangeTo == EWeaponSlot::None || !m_Weapons[m_NextWeaponTypeIdx])
		{
			m_CurWeaponTypeIdx  = NoneSlotIdx;
			m_NextWeaponTypeIdx = NoneSlotIdx;
			m_OwnerPlayer->SetHandState(EHandState::UnArmed);
			
			return false;
		}
		
		// 다음 무기가 있을 때
		m_OwnerPlayer->PlayAnimMontage(m_Weapons[m_NextWeaponTypeIdx]->GetDrawMontage());
		m_bIsCurrentlyChangingWeapon = true;
		return true;
	}
	
	/* 현재 무기를 착용중인 상황 */
	
	// TODO : 투척류 이미 쿠킹이 진행된 상태에서 다른 무기로 Swap시, 땅에 떨구는 예외처리를 해주었었음 (필요하다면 여기서도 처리를 해주어야 함)
	// TODO : 총을 들고 Aiming 상태였을 경우, 카메라 위치 원상복구 처리를 해주었음

	// 현재 무기 집어넣는 동작에 Notify 함수를 걸어둠 -> 다음 무기 Draw로 무기전환 처리가 이루어짐
	m_OwnerPlayer->PlayAnimMontage(m_Weapons[m_CurWeaponTypeIdx]->GetSheathMontage());
	m_bIsCurrentlyChangingWeapon = true;
	
	return true;
}

bool UC_EquippedComponent::ToggleArmed()
{
	const uint8 NoneSlotIdx = static_cast<uint8>(EWeaponSlot::None);
	
	// 현재 무기를 장착하지 않았고, 이전에 들고 있었던 무기도 없을 때(초기상태)
	if (m_CurWeaponTypeIdx == NoneSlotIdx && m_PrevWeaponTypeIdx == NoneSlotIdx)
		return false;
	
	/* 현재 들고 있는 무기가 있을 때 */
	if (m_CurWeaponTypeIdx != NoneSlotIdx && GetCurWeapon())
	{
		// 이전 무기 종류 저장 및 UnArmed 처리
		m_PrevWeaponTypeIdx = m_CurWeaponTypeIdx;
		return ChangeCurWeapon(EWeaponSlot::None);
	}
	
	/* 현재 들고 있는 무기가 없을 때 */
	return ChangeCurWeapon(static_cast<EWeaponSlot>(m_PrevWeaponTypeIdx));
}

void UC_EquippedComponent::OnSheathEnd()
{
	// Player HUD 업데이트
	if (APlayerController* PC = m_OwnerPlayer->GetController<APlayerController>())
	{
		if (AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD()))
			UIManager->GetMainHUDWidget()->ToggleAmmoInfoVisibility(false);
	}

	/* 무기를 바꾸는 도중에 SlotWeapon 장착 해제 예외 처리 -> Sheath 처리를 진행 중이던 무기가 Slot에서 빠졌을 때 */
	// 이 예외처리는 추후 좀비가 Player총기를 뺏을 수 있는 상황을 고려해서 넣어둠
	if (!GetCurWeapon())
	{
		m_CurWeaponTypeIdx = m_NextWeaponTypeIdx;

		// Swap할 다음 무기가 Valid하면, 다음 무기로 그대로 Swap 처리
		if (GetCurWeapon())
		{
			m_OwnerPlayer->PlayAnimMontage(GetCurWeapon()->GetDrawMontage());
			return;
		}

		/* 다음으로 바꿀 무기가 None일 때 */
		m_OwnerPlayer->SetHandState(EHandState::UnArmed);
		m_CurWeaponTypeIdx           = static_cast<uint8>(EWeaponSlot::None);
		m_NextWeaponTypeIdx          = static_cast<uint8>(EWeaponSlot::None);
		m_bIsCurrentlyChangingWeapon = false;
		return;
	}
	
	/* 일반적인 Case */
	
	// 현재 무기 무기집에 붙이기
	GetCurWeapon()->AttachToHolster(m_OwnerPlayer->GetMesh());
	
	// TODO : 여기에 총기류 예외처리를 넣어놨었음 (AimPress bool값 변경하는 처리가 들어가는 처리)
	
	m_CurWeaponTypeIdx = m_NextWeaponTypeIdx;
	
	if (!GetCurWeapon()) // 다음 무기 종류가 Valid하지 않은 상황(UnArmed 처리)
	{
		m_OwnerPlayer->SetHandState(EHandState::UnArmed);
		m_bIsCurrentlyChangingWeapon = false;
		return;
	}

	// 다음 무기가 Valid한 경우, 다음 무기 Draw (가장 Trivial한 case)
	m_OwnerPlayer->PlayAnimMontage(GetCurWeapon()->GetDrawMontage());
}

void UC_EquippedComponent::OnDrawEnd()
{
	m_bIsCurrentlyChangingWeapon = false;

	const uint8 NoneSlotIdx = static_cast<uint8>(EWeaponSlot::None);
	
	if (m_NextWeaponTypeIdx == NoneSlotIdx) return;
	
	// 무기를 바꾸는 도중에 SlotWeapon 장착 해제 예외 처리 -> 바꿔들 무기가 사라졌을 때
	if (!m_Weapons[m_NextWeaponTypeIdx])
	{
		m_NextWeaponTypeIdx = NoneSlotIdx;
		m_CurWeaponTypeIdx  = NoneSlotIdx;
		m_OwnerPlayer->SetHandState(EHandState::UnArmed);
		return;
	}
	
	m_Weapons[m_NextWeaponTypeIdx]->AttachToHand(m_OwnerPlayer->GetMesh());
	m_CurWeaponTypeIdx  = m_NextWeaponTypeIdx;
	m_NextWeaponTypeIdx = NoneSlotIdx;
}
