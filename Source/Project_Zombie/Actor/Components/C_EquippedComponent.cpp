#include "Actor/Components/C_EquippedComponent.h"

#include "C_InvenComponent.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "Actor/ItemActor/Weapon/Potion/C_PotionBase.h"
#include "Actor/ItemActor/Weapon/ThrowableWeapon/C_ThrowableWeaponBase.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Net/UnrealNetwork.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Controller/C_BasicPlayerController.h"
#include "UI/InvenUI/Upgrade/C_ItemUpgradeWidget.h"
#include "UI/InvenUI/C_InventoryWidget.h"
#include "Utility/C_Util.h"

UC_EquippedComponent::UC_EquippedComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
	
	// 각 Slot칸 nullptr로 미리 확보
	m_Weapons.SetNum(static_cast<uint8>(EWeaponSlot::Max));
}

void UC_EquippedComponent::BeginPlay()
{
	Super::BeginPlay();

	// 어디서 Set해주는거지?
	// 이게 왜 리슨서버에서 클라이언트는 nullptr가 나오는거지?
	m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
	
	if (!m_OwnerPlayer)
	{
		UC_Util::Print("From UC_EquippedComponent::BeginPlay : OwnerPlayer init failed!", FColor::Red, 10.f);
		UE_LOG(LogTemp, Error, TEXT("From UC_EquippedComponent::BeginPlay : OwnerPlayer init failed!"));
	}
}

void UC_EquippedComponent::SetSlotWeapon(EWeaponSlot TargetSlot, AC_WeaponBase* WeaponToEquip)
{
	if (TargetSlot == EWeaponSlot::None || TargetSlot == EWeaponSlot::Max)
	{
		UC_Util::Print("From UC_EquippedComponent::SetSlotWeapon : wrong TargetSlot received", FColor::Red, 10.f);
		return;
	}
	
	const uint8 TargetSlotIdx = static_cast<uint8>(TargetSlot); 
	
    // 들어온 슬롯의 이전 무기가 존재할 때, 이전 무기 해제 및 OwnerPlayer 초기화
    if (AC_WeaponBase* PrevSlotWeapon = m_Weapons[TargetSlotIdx])
    {
    	PrevSlotWeapon->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
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
        }

        return;
    }

	// 기존에 무기를 들고 있었다면 AttachToHolster 처리
	
	// Throwable의 경우 장착된 모습 보이지 않게끔 처리
    if (TargetSlot == EWeaponSlot::ThrowableWeapon)
        m_Weapons[TargetSlotIdx]->SetActorHiddenInGame(true);

	// m_Weapons[TargetSlotIdx]->SetRelativeTransformToInitial();
	
	// 무기에게 자신의 OwnerPlayer 세팅
	m_Weapons[TargetSlotIdx]->SetOwnerPlayer(m_OwnerPlayer);

	// 현재 들고 있는 무기의 종류에 따른 처리
	if (m_CurWeaponTypeIdx == TargetSlotIdx)
	{
		PRINT_LOCAL(GetWorld(), "Current holding weapon swapped to new Same SlotWeapon", FColor::Red, 10.f);
		m_Weapons[TargetSlotIdx]->AttachToHand(m_OwnerPlayer->GetMesh());
	}
    else m_Weapons[TargetSlotIdx]->AttachToHolster(m_OwnerPlayer->GetMesh());
}

void UC_EquippedComponent::UpdateWeaponData(EWeaponSlot _TargetWeapon, FName InItemRow)
{
	const uint8 Idx = static_cast<uint8>(_TargetWeapon);
	
	UC_ItemManager* ItemManager = m_OwnerPlayer->GetGameInstance()->GetSubsystem<UC_ItemManager>();

	if (!ItemManager) return;

	m_Weapons[Idx]->InitializeItemData(ItemManager->GetWeaponData(InItemRow));
	
	UpdateAmmoWidget();
	
	//AC_UIManager* UIManager = Cast<AC_UIManager>(Cast<AC_BasicPlayerController>(m_OwnerPlayer->GetController())->GetHUD());
	//
	//if (!UIManager) return;
	//
	//UC_InventoryWidget* InventoryWidget = UIManager->GetInventoryWidget();
	//
	//InventoryWidget->GetItemUpgradeWidget()->SetIsUpgrading(false);
	//
	//InventoryWidget->GetItemUpgradeWidget()->UpdateWidget();
}

void UC_EquippedComponent::Server_RequestSpawnEquippedActor_Implementation(int32 SlotIndex, const FInventoryEntry& ItemData)
{
	if (!GetWorld()) return;

	UC_ItemManager* ItemManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_ItemManager>();

	PRINT_LOCAL(GetWorld(), "ItemManager", FColor::Green, 5.f);

	if (!ItemManager) return;

	PRINT_LOCAL(GetWorld(), "SpawnEquippedActor", FColor::Green, 5.f);

	AC_WeaponBase* SpawnedWeapon = (ItemData.CurCount > 0) ? ItemManager->SpawnEquippedActor(ItemData.ItemRowName, m_OwnerPlayer) : nullptr;

	AC_WeaponBase* PrevWeapon = m_Weapons[SlotIndex];

	// 추가적인 부수처리가 같이 있어서 Server_SetSlotWeapon으로 수정
	Server_SetSlotWeapon(static_cast<EWeaponSlot>(SlotIndex), SpawnedWeapon);

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
}

void UC_EquippedComponent::Server_SetSlotWeapon_Implementation(EWeaponSlot _TargetSlot, AC_WeaponBase* _WeaponToEquip)
{
	PRINT_LOCAL(GetWorld(), "Server_SetSlotWeapon_Implementation", FColor::Red, 10.f);

	SetSlotWeapon(_TargetSlot, _WeaponToEquip); // 서버 환경에서의 SetSlotWeapon 처리

	// 서버 환경 자기자신일 때의 UI 업데이트
	if (m_OwnerPlayer->IsLocallyControlled())
	{
		UpdateAmmoWidget();
		return;
	}

	// 서버 플레이어가 아닌 경우, 대응되는 Client의 HUD 화면을 업데이트 처리
	
	if (!GetCurWeapon())
	{
		Client_UpdateAmmoWidget(FAmmoUIInfo());
	}
	else
	{
		// 현재 들고 있는 무기 상태에 맞게끔 UI 수정
		FAmmoUIInfo AmmoUIInfo{};
		GetCurWeapon()->SetAmmoUIInfo(AmmoUIInfo);
		
		Client_UpdateAmmoWidget(AmmoUIInfo);
	}
	
	// Multicast_SetSlotWeapon(_TargetSlot, _WeaponToEquip); // 클라이언트단의 SetSlotWeapon도 호출해줌으로써 동기화 처리
}

void UC_EquippedComponent::Client_UpdateWeaponData_Implementation(EWeaponSlot _TargetWeapon, FName InItemRow)
{
	UpdateWeaponData(_TargetWeapon, InItemRow);
}

bool UC_EquippedComponent::ChangeCurWeapon(EWeaponSlot _ChangeTo)
{
	const uint8 ChangeToIdx = static_cast<uint8>(_ChangeTo);
	const uint8 NoneSlotIdx = static_cast<uint8>(EWeaponSlot::None);

	// 현재 들고 있는 무기가 Valid하고, 해당 무기의 교체 모션이 이미 진행중이라면
	// 새로 들어온 교체 요청 처리 x
	if (m_Weapons[m_CurWeaponTypeIdx])
	{
		UAnimMontage* CurWeaponDrawMontage   = m_Weapons[m_CurWeaponTypeIdx]->GetDrawMontage();
		UAnimMontage* CurWeaponSheathMontage = m_Weapons[m_CurWeaponTypeIdx]->GetSheathMontage();
		UAnimInstance* OwnerAnimInstance     = m_OwnerPlayer->GetMesh()->GetAnimInstance(); 		

		if (OwnerAnimInstance->Montage_IsPlaying(CurWeaponDrawMontage) ||
			OwnerAnimInstance->Montage_IsPlaying(CurWeaponSheathMontage))
		{
			PRINT_LOCAL(GetWorld(), "IsCurrentlyChangingWeapon", FColor::MakeRandomColor(), 20.f);
			return false;
		}
	}

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
		PRINT_LOCAL(GetWorld(), "No Weapons on slot!", FColor::MakeRandomColor(), 20.f);
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
			Server_SetCurWeaponIdx(m_CurWeaponTypeIdx);
			
			m_NextWeaponTypeIdx = NoneSlotIdx;
			
			m_OwnerPlayer->SetHandState(EHandState::UnArmed);
			return false;
		}
		
		// 다음 무기가 있을 때
		const float Duration = m_OwnerPlayer->PlayAnimMontage(m_Weapons[m_NextWeaponTypeIdx]->GetDrawMontage());
		if (Duration == 0.f) return false; // MontagePriority에 의해 재생 처리가 제대로 안된 경우

		// 실질적인 Draw 처리 성공
		
		Server_PlayDrawMontage(m_Weapons[m_NextWeaponTypeIdx]); // 서버 쪽 DrawMontage play 요청 ( 나 재생함)
		return true;
	}
	
	/* 현재 무기를 착용중인 상황 */
	

	// 현재 무기 집어넣는 동작에 Notify 함수를 걸어둠 -> 다음 무기 Draw로 무기전환 처리가 이루어짐
	const float Duration = m_OwnerPlayer->PlayAnimMontage(GetCurWeapon()->GetSheathMontage());
	if (Duration == 0.f) return false; // Priority에 의한 Sheath 처리 거절
	
	Server_PlaySheathMontage(GetCurWeapon()); // 서버에 다른 Player들을 위한 자신의 Sheath 동작 처리되었다고 요청

	// 현재 무기의 Sheath가 시작된 경우, 초기화할 내역이 있다면 처리
	GetCurWeapon()->OnSheathStart();
	
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

void UC_EquippedComponent::LoadEquippedWeaponFromInven(int32 SlotIndex, const FInventoryEntry& ItemData)
{
	// 💡 PossessedBy는 100% 서버에서 실행되므로 Authority 체크 및 월드 체크
	if (!GetWorld() || !GetOwner() || !GetOwner()->HasAuthority()) return;
	if (SlotIndex < 0 || SlotIndex >= static_cast<int32>(EWeaponSlot::None)) return;

	// 이미 해당 슬롯에 무기가 있다면 중복 스폰 방지를 위해 리턴 또는 파괴 처리
	if (m_Weapons.IsValidIndex(SlotIndex) && m_Weapons[SlotIndex] != nullptr)
	{
		// 심리스 트래블 직후라면 기존 무기가 nullptr이겠지만, 혹시 모를 예외 처리
		return; 
	}

	//if (!m_OwnerPlayer)
	//{
	//	m_OwnerPlayer = Cast<AC_BasicPlayer>(GetOwner());
	//}
	
	UC_ItemManager* ItemManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	if (!ItemManager) return;

	UE_LOG(LogTemp, Log, TEXT("[Travel Restore] 슬롯 %d번에 무기 스폰 시작 (Row: %s)"), SlotIndex, *ItemData.ItemRowName.ToString());

	// TODO : m_Ownerplayer가 nullptr로 SpawnedWeapon이 Nullptr라 서버만 안되는 거였음.
	
	// 1. 아이템 매니저를 통해 새 레벨에 무기 액터 복구 스폰
	AC_WeaponBase* SpawnedWeapon = (ItemData.CurCount > 0) ? ItemManager->SpawnEquippedActor(ItemData.ItemRowName, m_OwnerPlayer) : nullptr;

	if (SpawnedWeapon)
	{
		// 2. 슬롯에 등록 및 물리적 부착(AttachToHand/Holster 등) 및 변수 복제 처리
		// RPC인 Server_SetSlotWeapon 대신, 서버 내부 로직인 SetSlotWeapon을 직접 호출합니다.
		SetSlotWeapon(static_cast<EWeaponSlot>(SlotIndex), SpawnedWeapon);
        
		// 추가로 필요한 초기화나 데이터 업데이트가 있다면 처리
		UpdateWeaponData(static_cast<EWeaponSlot>(SlotIndex), ItemData.ItemRowName);
	}
}

void UC_EquippedComponent::OnInventorySlotChanged(int32 SlotIndex, const FInventoryEntry& ItemData)
{
	if (SlotIndex < 0 || SlotIndex >= static_cast<int32>(EWeaponSlot::None)) return;   
	if (GetOwner() == nullptr) return;

	// 핵심: 만약 서버에서 실행 중이라면, RPC(Server_Request...)를 보낼 필요가 없습니다!
	// 서버 환경에서 인벤토리가 바뀐 거라면 직접 내부 구현 함수를 호출하면 됩니다.
	if (GetOwner()->HasAuthority())
	{
		// 서버 내부에서 일반적인 인벤토리 조작(예: 런타임 중 무기 교체 등)이 일어났을 때의 처리
		Server_RequestSpawnEquippedActor_Implementation(SlotIndex, ItemData);
		return;
	}

	// 오직 로컬 클라이언트 환경일 때만 서버에게 스폰을 요청(RPC)합니다.
	Server_RequestSpawnEquippedActor(SlotIndex, ItemData);
}

void UC_EquippedComponent::OnSheathEnd()
{
	if (!m_OwnerPlayer->IsLocallyControlled()) return;

	// 다음으로 들 무기가 있을 때에도 Widget Animation을 위해, AmmoVisibility false로 일괄 처리
	UI_MANAGER(GetWorld())->GetMainHUDWidget()->ToggleAmmoInfoVisibility(false);
	
	/* 무기를 바꾸는 도중에 SlotWeapon 장착 해제 예외 처리 -> Sheath 처리를 진행 중이던 무기가 Slot에서 빠졌을 때 */
	// 이 예외처리는 추후 좀비가 Player총기를 뺏을 수 있는 상황을 고려해서 넣어둠
	if (!GetCurWeapon())
	{
		m_CurWeaponTypeIdx = m_NextWeaponTypeIdx;
		Server_SetCurWeaponIdx(m_CurWeaponTypeIdx);
		
		// Swap할 다음 무기가 Valid하면, 다음 무기로 그대로 Swap 처리
		if (GetCurWeapon())
		{
			const float Duration = m_OwnerPlayer->PlayAnimMontage(GetCurWeapon()->GetDrawMontage());

			// 모종의 이유로(?) 다음 Weapon이 Valid하지만 MontagePriority에 의해 재생 처리가 안된 경우
			//  -> 이때에는 미아가 되어버림(현재 HandState를 강제 UnArmed 처리로 해주어야 한다)
			if (Duration == 0.f) 
			{
				UC_Util::Print("From UC_EquippedComponent::OnSheathEnd : Next DrawMontage Play failed! Cannot be possible on here!", FColor::Red, 10.f);
				
				m_OwnerPlayer->SetHandState(EHandState::UnArmed);
				m_CurWeaponTypeIdx           = static_cast<uint8>(EWeaponSlot::None);
				m_NextWeaponTypeIdx          = static_cast<uint8>(EWeaponSlot::None);
				Server_SetCurWeaponIdx(m_CurWeaponTypeIdx);
				return;
			}

			// 제대로 DrawMontage 재생 처리가 되었다면, 해당 Motion 재생
			Server_PlayDrawMontage(GetCurWeapon());
			return;
		}

		/* 다음으로 바꿀 무기가 None일 때 */
		m_OwnerPlayer->SetHandState(EHandState::UnArmed);
		m_CurWeaponTypeIdx           = static_cast<uint8>(EWeaponSlot::None);
		m_NextWeaponTypeIdx          = static_cast<uint8>(EWeaponSlot::None);
		Server_SetCurWeaponIdx(m_CurWeaponTypeIdx);
		return;
	}
	
	/* 일반적인 Case */
	
	// 현재 무기 무기집에 붙이기
	GetCurWeapon()->AttachToHolster(m_OwnerPlayer->GetMesh());
	Server_AttachToHolster(GetCurWeapon()); // 서버 쪽 환경에서도 Attaching 처리 (자동적으로 나머지 환경에서도 Attach 처리가 이루어진다)
	
	m_CurWeaponTypeIdx = m_NextWeaponTypeIdx;
	Server_SetCurWeaponIdx(m_CurWeaponTypeIdx);
	
	if (!GetCurWeapon()) // 다음 무기 종류가 Valid하지 않은 상황(UnArmed 처리)
	{
		m_OwnerPlayer->SetHandState(EHandState::UnArmed);
		return;
	}

	// 다음 무기가 Valid한 경우, 다음 무기 Draw (가장 Trivial한 case)
	const float Duration = m_OwnerPlayer->PlayAnimMontage(GetCurWeapon()->GetDrawMontage());
	if (Duration != 0.f) Server_PlayDrawMontage(GetCurWeapon()); 
}

void UC_EquippedComponent::OnDrawEnd()
{
	if (!m_OwnerPlayer->IsLocallyControlled()) return;

	const uint8 NoneSlotIdx = static_cast<uint8>(EWeaponSlot::None);
	
	if (m_NextWeaponTypeIdx == NoneSlotIdx) return;
	
	// 무기를 바꾸는 도중에 SlotWeapon 장착 해제 예외 처리 -> 바꿔들 무기가 사라졌을 때
	if (!m_Weapons[m_NextWeaponTypeIdx])
	{
		m_NextWeaponTypeIdx = NoneSlotIdx;
		m_CurWeaponTypeIdx  = NoneSlotIdx;
		Server_SetCurWeaponIdx(m_CurWeaponTypeIdx);
		m_OwnerPlayer->SetHandState(EHandState::UnArmed);
		return;
	}
	
	m_Weapons[m_NextWeaponTypeIdx]->AttachToHand(m_OwnerPlayer->GetMesh());
	Server_AttachToHand(m_Weapons[m_NextWeaponTypeIdx]); // 서버 쪽 무기도 Attach 처리 (Replicate 처리로 알아서 나머지 환경에서도 Attaching처리 된다)
	
	m_CurWeaponTypeIdx  = m_NextWeaponTypeIdx;
	m_NextWeaponTypeIdx = NoneSlotIdx;
	Server_SetCurWeaponIdx(m_CurWeaponTypeIdx);

	// 바뀐 무기로 AmmoInfo 정보 업데이트 처리
	if (GetCurWeapon()) GetCurWeapon()->UpdateAmmoInfoHUDForDrawEnd();
}

void UC_EquippedComponent::UpdateAmmoWidget()
{
	// 이 플레이어가 내가 플레이 중인 플레이어일 때, Ammo Info 관련 업데이트 처리
	if (!m_OwnerPlayer->IsLocallyControlled()) return;

	// 현재 들고 있는 무기가 없을 때
	if (!GetCurWeapon())
	{
		UI_MANAGER(GetWorld())->GetMainHUDWidget()->ToggleAmmoInfoVisibility(false);
		return;
	}
	
	// 현재 들고 있는 무기가 존재할 때, 해당 무기의 HUD 초기화 함수 사용
	GetCurWeapon()->UpdateAmmoInfoHUDForDrawEnd();
}

void UC_EquippedComponent::Client_UpdateAmmoWidget_Implementation(const FAmmoUIInfo& _AmmoUIInfo)
{
	// if (_AmmoUIInfo.Visible)

	if (_AmmoUIInfo.Visible)
		PRINT_LOCAL(GetWorld(), "RECEIVED CLIENT UPDATE AMMO_WIDGET : VISIBLE", FColor::Cyan, 20.f);
	else
		PRINT_LOCAL(GetWorld(), "RECEIVED CLIENT UPDATE AMMO_WIDGET : HIDDEN", FColor::Cyan, 20.f);
	
	UI_MANAGER(GetWorld())->GetMainHUDWidget()->ToggleAmmoInfoVisibility
	(
		_AmmoUIInfo.Visible,
		_AmmoUIInfo.FireMode,
		_AmmoUIInfo.MagazineAmmo,
		_AmmoUIInfo.LeftAmmoTotalCount
	);
}

void UC_EquippedComponent::Server_PlayDrawMontage_Implementation(AC_WeaponBase* _TargetWeapon)
{
	Multicast_PlayDrawMontage(_TargetWeapon);
}

void UC_EquippedComponent::Multicast_PlayDrawMontage_Implementation(AC_WeaponBase* _TargetWeapon)
{
	// LocalPlayer인 경우, 이미 Local 환경에서 모든 처리를 끝낸 상황
	if (m_OwnerPlayer->IsLocallyControlled()) return;
	
	if (!_TargetWeapon) return;
	m_OwnerPlayer->PlayAnimMontage(_TargetWeapon->GetDrawMontage());
}

void UC_EquippedComponent::Server_PlaySheathMontage_Implementation(AC_WeaponBase* _TargetWeapon)
{
	Multicast_PlaySheathMontage(_TargetWeapon);
}

void UC_EquippedComponent::Multicast_PlaySheathMontage_Implementation(AC_WeaponBase* _TargetWeapon)
{
	// LocalPlayer인 경우, 이미 Local 환경에서 모든 처리를 끝낸 상황
	if (m_OwnerPlayer->IsLocallyControlled()) return;
	
	if (!_TargetWeapon) return;
	m_OwnerPlayer->PlayAnimMontage(_TargetWeapon->GetSheathMontage());
}

void UC_EquippedComponent::Server_AttachToHolster_Implementation(AC_WeaponBase* _TargetWeapon)
{
	if (_TargetWeapon) _TargetWeapon->AttachToHolster(m_OwnerPlayer->GetMesh());
}

void UC_EquippedComponent::Server_AttachToHand_Implementation(AC_WeaponBase* _TargetWeapon)
{
	if (_TargetWeapon) _TargetWeapon->AttachToHand(m_OwnerPlayer->GetMesh());
}

void UC_EquippedComponent::Server_SetCurWeaponIdx_Implementation(uint8 _NewIdx)
{
	m_CurWeaponTypeIdx = _NewIdx;
}

void UC_EquippedComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 리플리케이트 하고싶은 멤버를 등록 여기서
	DOREPLIFETIME(UC_EquippedComponent, m_Weapons);
	DOREPLIFETIME(UC_EquippedComponent, m_CurWeaponTypeIdx);
	
	// DOREPLIFETIME(UC_EquippedComponent, m_bIsCurrentlyChangingWeapon);
}
