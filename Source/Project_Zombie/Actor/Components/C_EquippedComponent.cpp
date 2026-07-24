#include "Actor/Components/C_EquippedComponent.h"

#include "C_InvenComponent.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "Actor/ItemActor/Weapon/ThrowableWeapon/C_ThrowableWeaponBase.h"
#include "GameModeAndManager/C_ItemManager.h"
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
	
	//
	//EquipInvenComp->SetMaxSlots(static_cast<int32>(EWeaponSlot::Max));
	
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
	if (TargetSlot == EWeaponSlot::None || TargetSlot == EWeaponSlot::Max)
	{
		UC_Util::Print("From UC_EquippedComponent::SetSlotWeapon : wrong TargetSlot received", FColor::Red, 10.f);
		return nullptr;
	}
	
	const uint8 TargetSlotIdx = static_cast<uint8>(TargetSlot); 
	AC_WeaponBase* PrevSlotWeapon = m_Weapons[TargetSlotIdx];

    // 들어온 슬롯의 이전 무기가 존재할 때, 이전 무기 해제 및 OwnerPlayer 초기화
    if (PrevSlotWeapon)
    {
    	m_Weapons[TargetSlotIdx]->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
    	PrevSlotWeapon->SetOwnerPlayer(nullptr);
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

	// 무기에게 자신의 OwnerPlayer 세팅
	m_Weapons[TargetSlotIdx]->SetOwnerPlayer(m_OwnerPlayer);
	
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

void UC_EquippedComponent::SetupInventoryComponent(UC_InvenComponent* InInvenComp)
{
	// 이미 같은 인벤토리가 바인딩되어 있다면 중복 처리 방지
	if (BoundInvenComp.Get() == InInvenComp)
	{
		return;
	}

	// 혹시 기존에 묶인 게 있다면 해제
	ClearInventoryComponent();

	if (InInvenComp)
	{
		InInvenComp->OnInventorySlotChanged.AddDynamic(this, &UC_EquippedComponent::OnInventorySlotChanged);
		BoundInvenComp = InInvenComp;
	}
}

void UC_EquippedComponent::ClearInventoryComponent()
{
	if (BoundInvenComp.IsValid())
	{
		BoundInvenComp->OnInventorySlotChanged.RemoveDynamic(this, &UC_EquippedComponent::OnInventorySlotChanged);
	}
	BoundInvenComp.Reset();
}

void UC_EquippedComponent::OnInventorySlotChanged(int32 SlotIndex, const FInventoryEntry& ItemData)
{
	if (SlotIndex < 0 || SlotIndex >= static_cast<int32>(EWeaponSlot::None)) return;	
	
	if (m_OwnerPlayer && !m_OwnerPlayer->HasAuthority()) return;
	
	// 들어오는 장비가 실제 장비가 있는지 없는 확인
	// 1. RowName이 NAME_NONE이 드롭되서 들어올 일은 없지만
	// 2. 장비창의 아이템을 인벤의 빈 슬롯에 드롭하면 들어올 수 있음.
	// 3. RowName == NAME_None이면 아이템 해제하고 빈칸으로 만들어야 함.
	// 4. 실제 존재하는 장비가 들어오는 경우
	// 5. 해당 아이템을 ItemManager로 생성
	// 6. SetSlotWeapon으로 장착
	// 7. 이 때 장착했던 장비가 있었으면 Destroy
	// 8. Destroy되는 장비의 정보(FInventoryEntry)는 인벤에서 Swap으로 이미 들어온 장비와 위치가 바뀜.
	// 9. 결론은 여기 매개변수로 해당 슬롯 장비를 소환해서 장착하고 탈착된 장비는 Destroy한다.
	// 10. 그러기 위해 해당 무기를 ItemManager에서 생성하는 함수를 만들어야 한다.
	
	
	if (!GetWorld()) return;
		
	UC_ItemManager* ItemManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_ItemManager>();
		
	if (!ItemManager) return;
	
	AC_WeaponBase* SpawnedWeapon = ItemManager->SpawnEquippedActor(ItemData.ItemRowName, m_OwnerPlayer);
	
	AC_WeaponBase* PrevWeapon = SetSlotWeapon(static_cast<EWeaponSlot>(SlotIndex), SpawnedWeapon);
	
	if (!PrevWeapon) return;
	
	AC_ThrowableWeaponBase* ThrowableWeapon = Cast<AC_ThrowableWeaponBase>(PrevWeapon);
	
	if (ThrowableWeapon)
	{
		// ThrowableWeaponBase::OnThrowThrowable에서 투척류 숫자 차감하고 업데이트하고 있음.
		// ThrowableWeapon은 투척한거면 여기서 삭제하면 안됨.
		if (static_cast<int32>(EThrowableState::RemovePin) < static_cast<int32>(ThrowableWeapon->GetThrowableState()))
		{
			return;
		}
	}
	
	
	PrevWeapon->Destroy();
		
	
	
	// 빈슬롯, 장비 해제 처리 확인하기
	// 동일 무기 교체시 데이터만 교체하는 방식으로 하면 좋음
	// 무기 교체? 장착? 사운드?
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
