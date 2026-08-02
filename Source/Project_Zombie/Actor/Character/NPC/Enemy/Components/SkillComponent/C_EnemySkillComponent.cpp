// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemySkillComponent.h"

#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillBase.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"

// 비동기 로딩 관련 헤더
#include "Engine/AssetManager.h"

// 팀
#include "GenericTeamAgentInterface.h"

// 키즈멧
#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "GameModeAndManager/C_UIManager.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"

UC_EnemySkillComponent::UC_EnemySkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SetIsReplicatedByDefault(true);

	for (int32 i = 0; i < (int32)ESkillSlot::END; i++)
	{
		m_SkillSlots.Add(FSkillSlotInfo{ (ESkillSlot)i, });
	}

}

void UC_EnemySkillComponent::BeginPlay()
{
	Super::BeginPlay();

	// 주인 캐릭터 초기화
	m_OwnerEnemy = Cast<AC_BasicEnemy>(GetOwner());
	if (!m_OwnerEnemy)
	{
		// 캐릭터 Base인 객체만 일단 Skill 프레임워크를 사용할 수 있다고 가정
		UC_Util::Print("From UC_EnemySkillComponent::BeginPlay : Please attach SkillComponent to Enemy based class!", FColor::Red, 10.f);
	}
	
	InitializeSkills();
}

void UC_EnemySkillComponent::InitializeSkills()
{
	for (FSkillSlotInfo& Info : m_SkillSlots)
	{
		// DataAsset 로드
		Info.LoadedSkillData = Info.SkillData.LoadSynchronous();

		if (!Info.LoadedSkillData)
			continue;

		// SkillClass 확인
		if (!Info.LoadedSkillData->SkillClass)
			continue;

		// Skill 객체 생성
		Info.SkillInstance = NewObject<UC_EnemySkillBase>(this, Info.LoadedSkillData->SkillClass);
	}
}


void UC_EnemySkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

bool UC_EnemySkillComponent::UseSkill(ESkillSlot _Slot)
{

	// 이미 다른 스킬을 사용중일 때
	if (bUsingSkill) return false;

	AC_BasicEnemy* Owner = Cast<AC_BasicEnemy>(GetOwner());
	if (!Owner) return false;

	if (!m_SkillSlots.IsValidIndex(static_cast<int32>(_Slot)))
		return false;

	const int32 TargetSlotIdx  = static_cast<int32>(_Slot);
	const FSkillSlotInfo& Info = m_SkillSlots[TargetSlotIdx];
	
	if (!IsValid(Info.LoadedSkillData))
	{
		UC_Util::Print("From UC_EnemySkillComponent::UseSkill : TargetSlot skill not loaded!", FColor::Red, 10.f);
		return false;
	}

	if (!IsValid(Info.SkillInstance))
		return false;

	// 현재 해당 스킬을 사용할 수 없는 상황
	int32 PlayedMontageSection{}; // 스킬 사용 성공 시, Montage의 어느 섹션이 재생되었는지 구해줌
	if (!Info.SkillInstance->Activate(Owner, Info.LoadedSkillData, PlayedMontageSection))
		return false;


	// 스킬 사용 성공
	m_CurSkillData = Info.LoadedSkillData;	// 현재 사용중인 Skill 세팅
	bUsingSkill    = true;					// 스킬 사용중으로 설정

	StartCooldown(_Slot);
	
	// 현재 서버환경에서 실행되는 중, 스킬 정상 발동되었으니 Multicast를 통해 나머지 게스트 환경에서도 스킬 동작 나오도록 RPC 호출 처리
	Multicast_ImitateUseSkill(_Slot, PlayedMontageSection);
	
	// 클라이언트 환경에서 현재 동작중인 최신 스킬 슬롯을 서버 쪽에서도 들고 있게끔 처리(이 값은 딱히 Replicate 처리까지는 하지 않음)
	m_CurImitatingSkillSlot = _Slot;  
	
	return true;
}

void UC_EnemySkillComponent::OnAN_EndSkill()
{
	bUsingSkill = false;
	m_CurSkillData = nullptr;

	m_SkillEndDelegate.Broadcast(Cast<AC_BasicEnemy>(GetOwner()));
}

void UC_EnemySkillComponent::EndSkillManually()
{
	// AnimNotify를 통해(AnimMontage 종료시점을 통해) 호출된 것이 아니기 때문에 Montage 모션을 직접 Stop 시켜주어야 한다.
	// 현재 Skill이 Valid하고, 해당 Skill의 모션이 끝나지 않았다면 끊어줌
	if (m_CurSkillData && m_OwnerEnemy->GetMesh()->GetAnimInstance()->Montage_IsPlaying(m_CurSkillData->Montage))
		m_OwnerEnemy->StopAnimMontage(m_CurSkillData->Montage);
	
	Multicast_ImitateEndSkillManually(m_CurImitatingSkillSlot);
	
	m_CurImitatingSkillSlot = ESkillSlot::END;

	// 나머지 처리는 기존의 OnEndSkill 처리와 같음
	OnAN_EndSkill();
}

bool UC_EnemySkillComponent::CanUseSkill(ESkillSlot _Slot) const
{
	if (bUsingSkill)
		return false;

	const int32 SlotIndex = static_cast<int32>(_Slot);
	if (!m_SkillSlots.IsValidIndex(SlotIndex))
		return false;

	// 스킬 정보 가져오기
	const FSkillSlotInfo& Info = m_SkillSlots[SlotIndex];
	if (!IsValid(Info.LoadedSkillData))
		return false;
	if (!IsValid(Info.SkillInstance))
		return false;

	const UWorld* World = GetWorld();
	if (!IsValid(World))
		return false;

	// 다음 스킬사용 시간 찾기
	const float* NextUsableTime = m_mapSkillCoolTime.Find(Info.LoadedSkillData->GetPrimaryAssetId());

	// 아직 한번도 사용하지 않은 스킬이라면
	if (!NextUsableTime)
		return true;

	return GetWorld()->GetTimeSeconds() >= *NextUsableTime;
}

void UC_EnemySkillComponent::StartCooldown(ESkillSlot _Slot)
{
	const int32 SlotIndex = static_cast<int32>(_Slot);

	if (!m_SkillSlots.IsValidIndex((SlotIndex)))
		return;

	const FSkillSlotInfo& Info = m_SkillSlots[SlotIndex];

	if (!IsValid(Info.LoadedSkillData))
		return;

	// 현재 시간 + 스킬의 쿨타임 = 다음 스킬사용 가능시간 계산
	const float NextUsableTime = GetWorld()->GetTimeSeconds() + Info.LoadedSkillData->CoolTime;

	m_mapSkillCoolTime.Add(Info.LoadedSkillData->GetPrimaryAssetId(), NextUsableTime);
}

float UC_EnemySkillComponent::GetRemainingCooldown(ESkillSlot _Slot) const
{
	const UWorld* World = GetWorld();

	if(!IsValid(World))
		return 0.f;

	const int32 SlotIndex = static_cast<int32>(_Slot);

	if (!m_SkillSlots.IsValidIndex(SlotIndex))
		return 0.f;

	const FSkillSlotInfo& Info = m_SkillSlots[SlotIndex];

	if (!IsValid(Info.LoadedSkillData))
		return 0.f;

	const FPrimaryAssetId SkillId = Info.LoadedSkillData->GetPrimaryAssetId();

	const float* NextUsableTime = m_mapSkillCoolTime.Find(SkillId);

	// 한번도 사용하지 않은 경우
	if (!NextUsableTime)
		return 0.f;

	const float CurrentTime = World->GetTimeSeconds();

	// 쿨타임이 끝났으면 음수가 나올 수 있으므로 최소값 0으로 제한
	return FMath::Max(0.f, *NextUsableTime - CurrentTime);

}

void UC_EnemySkillComponent::Fire()
{
	if (!m_CurSkillData)
		return;
	for (FSkillSlotInfo& Info : m_SkillSlots)
	{
		if (Info.LoadedSkillData == m_CurSkillData)
		{
			if (!Info.SkillInstance)
				return;

			Info.SkillInstance->Fire(Cast<AC_BasicEnemy>(GetOwner()), Info.LoadedSkillData);
			return;
		}
	}
}

float UC_EnemySkillComponent::GetSkillRange(ESkillSlot _Slot) const
{
	const int32 TargetIdx = static_cast<int32>(_Slot);
	
	if (!m_SkillSlots.IsValidIndex(TargetIdx) || !m_SkillSlots[TargetIdx].LoadedSkillData) return 0.f;
	
	return m_SkillSlots[TargetIdx].LoadedSkillData->Range;
}

float UC_EnemySkillComponent::GetCurSkillDamage() const
{
	if (!m_CurSkillData)
	{
		UC_Util::Print("[UC_EnemySkillComponent::GetCurSkillDamage] : CurSkillData nullptr", FColor::Red, 10.f);
		return 0.f;
	}
	
	return m_CurSkillData->Damage;
}

void UC_EnemySkillComponent::Multicast_ImitateUseSkill_Implementation(ESkillSlot _ImitatingSkillSlot, int32 _PlayedMontageSection)
{
	// 서버 환경의 UseSkill 따라하기 요청처리는 무시 (자기자신이 보낸 요청이고, 자기자신은 이미 UseSkill 동작을 발현한 상태)
	if (!m_OwnerEnemy || m_OwnerEnemy->IsLocallyControlled()) return;
	
	// 나머지 쩌리 클라이언트 중생들을 위한 UseSkill 동작 따라하기 요청 처리

	const int32 SkillSlotIdx = static_cast<int32>(_ImitatingSkillSlot);
	
	if (!m_SkillSlots.IsValidIndex(SkillSlotIdx))
	{
		PRINT_LOCAL(GetWorld(), "From UC_EnemySkillComponent::Multicast_ImitateUseSkill_Implementation : Invalid SkillSlot received!", FColor::Red, 10.f);
		return;
	}
	
	// 스킬 동작 따라하기
	UAnimMontage* TargetSkillMontage = m_SkillSlots[SkillSlotIdx].LoadedSkillData->Montage;
	const FName TargetSectionName    = TargetSkillMontage->GetSectionName(_PlayedMontageSection);
	const float Duration = m_OwnerEnemy->PlayAnimMontage(TargetSkillMontage, 1.f, TargetSectionName);

	// 제대로 재생 처리 되었다면, 현재 재생 중인 Skill 동작 Slot을 저장
	// Manual 하게 동작이 끊긴 처리 Multicast 수신했을 경우, 더블 체크로 현재 수행중인 동작을 끊어버릴 예정
	if (Duration > 0.f) m_CurImitatingSkillSlot = _ImitatingSkillSlot;
}

void UC_EnemySkillComponent::Multicast_ImitateEndSkillManually_Implementation(ESkillSlot _TargetSkillSlot)
{
	if (!m_OwnerEnemy || m_OwnerEnemy->IsLocallyControlled()) return;

	// 다른 동작을 처리 중
	if (m_CurImitatingSkillSlot != _TargetSkillSlot) return;

	const int32 TargetIdx = static_cast<int32>(_TargetSkillSlot);
	if (!m_SkillSlots.IsValidIndex(TargetIdx)) return;
	
	if (m_SkillSlots[TargetIdx].LoadedSkillData && m_SkillSlots[TargetIdx].LoadedSkillData->Montage)
	{
		UAnimInstance* OwnerAnimInst = m_OwnerEnemy->GetMesh()->GetAnimInstance();
		
		if (OwnerAnimInst->Montage_IsPlaying(m_SkillSlots[TargetIdx].LoadedSkillData->Montage))
			m_OwnerEnemy->StopAnimMontage(m_SkillSlots[TargetIdx].LoadedSkillData->Montage);
	}
	
	// Imitating 처리 중인 동작이 없다고 판단
	m_CurImitatingSkillSlot = ESkillSlot::END;
}