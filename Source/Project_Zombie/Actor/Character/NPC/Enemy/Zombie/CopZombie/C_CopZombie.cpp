// Fill out your copyright notice in the Description page of Project Settings.


#include "C_CopZombie.h"

#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/Components/C_InvenComponent.h"
#include "Actor/Components/ItemLinkComponent/C_ItemLinkComponent.h"
#include "Actor/ItemActor/Weapon/C_WeaponBase.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "Actor/ItemActor/Weapon/WeaponComponent/GunComponent/AIGunUsageComponent/C_AIGunUsageComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/GameSession.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Utility/C_Util.h"


AC_CopZombie::AC_CopZombie()
	: Super(EZombieType::CopZombie)
{
	PrimaryActorTick.bCanEverTick = false;
	
	m_GrabRangeCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("GrabRangeCollider"));
	m_GrabRangeCollider->SetupAttachment(GetRootComponent());
	
	m_NormalAttackCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("NormalAttackCollider"));
	m_NormalAttackCollider->SetupAttachment(GetMesh(), TEXT("RightForeArm"));
	AddNormalAttackCollider(m_NormalAttackCollider);
}

void AC_CopZombie::BeginPlay()
{
	Super::BeginPlay();
	// m_GrabRangeCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (!IsLocallyControlled())
	{
		m_GrabRangeCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return; // Server쪽 좀비인 경우에만, CollisionHandling 처리
	}
	
	m_GrabRangeCollider->OnComponentBeginOverlap.AddDynamic(this, &AC_CopZombie::OnGrabRangeColliderBeginOverlap);
	m_GrabRangeCollider->OnComponentEndOverlap.AddDynamic(this, &AC_CopZombie::OnGrabRangeColliderEndOverlap);
}

void AC_CopZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_CopZombie::OnANSGrabStart()
{
	if (!IsLocallyControlled()) return;
	
	m_GrabRangeEnteredPlayers.Empty();
	m_GrabRangeCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AC_CopZombie::OnANSGrabEnd()
{
	if (!IsLocallyControlled()) return;

	m_GrabRangeCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// ANS가 끊기는 것이 -> Abort Task에 의해 끊길 수도 있음 -> 따라서 자체적으로 BTTask_GrabMainWeapon에서 EndSkill 시, Skill 성공 여부에 따라서 처리해줄 것
}

void AC_CopZombie::OnGrabRangeColliderBeginOverlap
(
	UPrimitiveComponent* OverlappedComponent,
	AActor*				 OtherActor,
	UPrimitiveComponent* OtherComp,
	int32				 OtherBodyIndex,
	bool				 bFromSweep,
	const FHitResult&	 SweepResult
)
{
	// Client 쪽은 Event 바인딩 처리 자체를 안해서 검사하지 않아도 됨
	
	if (AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor))
		m_GrabRangeEnteredPlayers.Add(Player);
}

void AC_CopZombie::OnGrabRangeColliderEndOverlap
(
	UPrimitiveComponent* OverlappedComponent,
	AActor*				 OtherActor,
	UPrimitiveComponent* OtherComp,
	int32				 OtherBodyIndex
)
{
	// Client 쪽은 Event 바인딩 처리 자체를 안해서 검사하지 않아도 됨
	
	if (AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor))
		m_GrabRangeEnteredPlayers.Remove(Player);
}

bool AC_CopZombie::EquipWeapon(AC_GunBase* _StolenWeapon)
{
	if (!_StolenWeapon) return false;
	if (m_EquippedGun) return false; // 이미 빼앗은 무기가 존재
	
	// 손에 부착 시도
	if (!_StolenWeapon->GetAIGunUsageComponent()->AttachToHand(GetMesh()))
		return false;
	
	// 부착 성공, State 변화 및 EquippedWeapon 저장
	m_EquippedGun    = _StolenWeapon;
	
	UC_ItemLinkComponent* LinkComp = m_EquippedGun->GetLinkComp();
	
	if (LinkComp)
	{
		// Entry 정보 복사 TODO : LinkComp->GetItemEntryPtr()로 원본 포인터를 받아 오는게 나은가?
		FInventoryEntry* TempEntry = LinkComp->GetItemEntryPtr();
		
		// 빈 Entry 생성.
		FInventoryEntry sub = FInventoryEntry();
		sub.SlotIndex = 0;
		
		// TempEntry가 비어 있지 않다면 Cop에게 복사
		if (TempEntry)
		{
			Entry = *TempEntry;
			Entry.LockedByPlayerID = INDEX_NONE;
			
			TempEntry->Clear();
			
			sub.SlotIndex = Entry.SlotIndex;// 사실 99퍼 0임.
			
			UC_InvenComponent* LinkOwningInvenComp = LinkComp->GetOwningInvenComp();
			
			// 무기를 빼앗긴 Player의 UI 업데이트.
			if (LinkOwningInvenComp)
			{
				
				AC_BasicPlayer* OriginPlayer = Cast<AC_BasicPlayer>(LinkOwningInvenComp->GetOwner());
				
				if (OriginPlayer)
					OriginPlayer->GetInvenComponent()->InitInvenItemAt(Entry.SlotIndex);
				
				//if (OriginPlayer && OriginPlayer->IsLocallyControlled())
				//	LinkOwningInvenComp->OnInventorySlotChanged.Broadcast(LinkComp->GetSlotIndex(), sub);
				//else
				//	LinkOwningInvenComp->MarkSlotDirty(Entry.SlotIndex);
			}
		}
		else
		{
			// 이 상황이 나오면 사실 안됨.
			sub.ItemRowName = _StolenWeapon->GetWeaponRowName();
			sub.CurCount = 1;
			Entry = sub;
		}
		
		Entry.SlotIndex = -1;
	}
	
	m_CopZombieState = ECopZombieState::WeaponEarned; // ABP 무기 자세로 자세전환
	return true;
}

void AC_CopZombie::DropWeapon()
{
	// 현재 들고있는 무기가 없을 때
	if (!m_EquippedGun) return;
	//if (!m_EquippedGun->GetAIGunUsageComponent()->DetachFromHand()) return;
	
	// C_ItemPickUp을 스폰해서 뿌려주면 됨. 이 때 AC_Weapon의 정보를 통해 데이터 테이블, 강화 테이블을 통해 역산해서 FInventoryEntry만들어야 함.
	
	m_EquippedGun = nullptr;
	
	UC_ItemManager* ItemManager = GetGameInstance()->GetSubsystem<UC_ItemManager>();
	
	if (!ItemManager) return;
	
	ItemManager->DropItemByPlayer(Entry, this);
	
	Entry = FInventoryEntry();
}

void AC_CopZombie::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AC_CopZombie, m_EquippedGun);
	DOREPLIFETIME(AC_CopZombie, m_CopZombieState);
}
