// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkillComponent/C_EnemySkillComponent.h"
#include "Components/StatComponent/C_EnemyStatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameModeAndManager/C_GameMode_GameLv.h"
#include "Net/UnrealNetwork.h"
#include "BrainComponent.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "GameModeAndManager/C_ItemManager.h"
#include "GameModeAndManager/C_ZombieManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "Item/DataAsset/C_DropTableDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Damage.h"
#include "Utility/C_Util.h"
#include "Zombie/NurseZombie/C_NurseZombie.h"
#include "Zombie/Controller/C_ZombieController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

const int8 AC_BasicEnemy::s_MaxHealRequestRegisterCount = 2;


AC_BasicEnemy::AC_BasicEnemy()
{
	// Replication 설정
	// SetReplicates(true); -> 이걸 걸면 오히려 뚝뚝 끊겨보이는데 왜지?
	
	// 스탯 컴포넌트 추가
	m_StatComponent = CreateDefaultSubobject<UC_EnemyStatComponent>(TEXT("StatComponent"));

	// 스킬 컴포넌트 추가
	m_SkillCom = CreateDefaultSubobject<UC_EnemySkillComponent>(TEXT("SkillComponent"));
	
	m_HealedEffectNGComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HealedEffectNGComponent"));
	m_HealedEffectNGComponent->SetAutoDestroy(false); // NS의 Loop가 Once일 경우, NGComponent Destroy 처리 방지
	m_HealedEffectNGComponent->SetupAttachment(GetRootComponent());
	
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> HealedEffect
	(TEXT("/Script/Niagara.NiagaraSystem'/Game/DongHyun/Effect/EnemyHealed.EnemyHealed'"));
	
	if (HealedEffect.Succeeded())
		m_HealedEffectNGComponent->SetAsset(HealedEffect.Object.Get());

	GetCharacterMovement()->RotationRate = FRotator(0.f, 240.f, 0.f);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	
}

void AC_BasicEnemy::ResetEnemyForPoolSpawn()
{
	// 좀비 상태는 서버에서만 처리
	if (!HasAuthority())
		return;

	// 죽음 상태 초기화
	m_DeadRepData.bDead = false;
	m_DeadRepData.DeadMontageIndex = INDEX_NONE;

	// 이전의 죽음 타이머가 남아있다면 제거
	GetWorldTimerManager().ClearTimer(m_DeadRemainTimer);

	// 몽타주 종료
	StopAnimMontage();

	// HP 초기화
	if (IsValid(m_StatComponent))
	{
		m_StatComponent->SetCurHP(m_StatComponent->GetStat(StatName::MaxHP));
	}

	// 이동 복구
	if (UCharacterMovementComponent* MoveCom = GetCharacterMovement())
	{
		MoveCom->SetMovementMode(EMovementMode::MOVE_Walking);

		MoveCom->StopMovementImmediately();
	}

	// 힐 요청 상태 초기화
	m_HealRequestRegisterCount = 0;

	// 힐 이펙트 초기화
	if (IsValid(m_HealedEffectNGComponent))
	{
		m_HealedEffectNGComponent->DeactivateImmediate();
	}

	// 죽음 상태 변경을 클라에 빠르게 전달
	ForceNetUpdate();
}

int32 AC_BasicEnemy::SelectHitMontageIndex() const
{
	if (m_HitMontage.IsEmpty())
		return INDEX_NONE;

	return FMath::RandRange(0, m_HitMontage.Num() - 1);
}

void AC_BasicEnemy::PlayHitAnimation(int32 _Idx)
{
	if (!m_HitMontage.IsValidIndex(_Idx))
		return;

	UAnimMontage* HitMontage = m_HitMontage[_Idx];

	if (!IsValid(HitMontage))
		return;

	PlayAnimMontage(HitMontage);
}

void AC_BasicEnemy::Multicast_PlayHit_Implementation(int32 _HitMontageIdx)
{
	PlayHitAnimation(_HitMontageIdx);
	PlayHitSound();
}

void AC_BasicEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 서버가 결정한 죽음 상태와 몽타주 인덱스를 클라이언트에 복제
	DOREPLIFETIME(AC_BasicEnemy, m_DeadRepData);
}

void AC_BasicEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		// 죽었을 때 처리할 함수 Delegate 구독 처리 (죽은 이후로도 다음에 Spawn 처리가 될 수 있기 때문에 구독 해지는 처리하지 않는다)
		m_StatComponent->OnCurHPReachedZeroDelegate.AddUObject(this, &AC_BasicEnemy::OnDead);

		// IncreaseCurHP 정상 처리 시(힐 받은 처리로 판단) -> 힐 받은 Effect 활성화 함수 Delegate 구독 처리
		m_StatComponent->OnIncreaseCurHPDelegate.AddUObject(this, &AC_BasicEnemy::OnHPIncreased);
		
		m_ZombieController = GetController<AC_ZombieController>();

		if (UGameInstance* GI = GetGameInstance())
		{
			m_ItemManager = GI->GetSubsystem<UC_ItemManager>();
		}
	}
	/*else // 클라이언트 환경
	{
		// 클라이언트단 화면에서는 Controller가 없기에, Controller Rotation (0, 0, 0) 값을 사용ㄴ
		// 따라서 끊겨보이는 버그가 있었음
		bUseControllerRotationYaw                             = false;
		GetCharacterMovement()->bOrientRotationToMovement     = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate                  = FRotator(0.f, 360.f, 0.f);
		
	}*/

	// HealEffect 재생 속도 조절
	m_HealedEffectNGComponent->SetCustomTimeDilation(2.f);
	m_HealedEffectNGComponent->DeactivateImmediate();
	
	// 바닥면으로 위치 맞추기
	m_HealedEffectNGComponent->SetRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
}

float AC_BasicEnemy::TakeDamage
(
	float				_DamageAmount,
	FDamageEvent const& _DamageEvent,
	AController*		_EventInstigator,
	AActor*				_DamageCauser
)
{
	const float DamageAmount = Super::TakeDamage(_DamageAmount, _DamageEvent, _EventInstigator, _DamageCauser);
	if (DamageAmount <= 0.f) return 0.f; // Damage가 들어오지 않음 (클라이언트단, TakeDamage 로컬 호출인 경우에 그럴 수 있음 -> 알아서 서버 쪽으로 Damage 입은 사실 전달)
	
	// 죽지 않은 경우에만 피격모션 재생
	if (IsValid(m_StatComponent) && m_StatComponent->GetCurHP() > 0.f)
	{
		const int32 HitIdx = SelectHitMontageIndex();

		if (HitIdx != INDEX_NONE)
		{
			Multicast_PlayHit(HitIdx);
		}
	}

	/* PerceptionComponent에 Damage를 받았다고 보고 처리 */
	
	if (ACharacter* DamageInstigator = _EventInstigator->GetCharacter())
	{
		const FVector DamageInstigatorPos = (DamageInstigator != nullptr) ? DamageInstigator->GetActorLocation() : this->GetActorLocation();
		
		UAISense_Damage::ReportDamageEvent
		(
			GetWorld(),				 // 히트 이벤트가 발생한 월드 
			this,					 // 맞은 놈 
			DamageInstigator,		 // 때린 놈 
			DamageAmount,			 // 최종 데미지
			DamageInstigatorPos,	 // 때린놈 위치 
			this->GetActorLocation() // 맞은놈 위치
		);
	}

	// 사망 했을 경우
	if (m_StatComponent->IsCurHPZero())
	{
		// 마지막으로 죽인 Player에게 Kill 수 업데이트 처리를 하라고 보내줄 것
		if (AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_EventInstigator->GetPawn()))
			Player->Multicast_IncreaseKillCount();
	}
	
	/* 힐 요청 처리 관련 */

	// 이미 힐 요청 최대 등록 횟수를 기록
	if (m_HealRequestRegisterCount >= s_MaxHealRequestRegisterCount) return DamageAmount;
	
	// 현재 생명력 Ratio 50% ~ 70% 랜덤 수치 이하면, 가능한 힐러 좀비에게 힐 요청 시도
	// TODO : 이거 요청 빈도가 너무 높으면 여기서 병목 생길수도 있음 -> 추후 최적화할 때 고려할 것
	if (m_StatComponent->GetCurHPRatio() < FMath::RandRange(0.5f, 0.7f))
	{
		for (AC_Zombie* Zombie : ZOMBIE_MANAGER(this)->GetActiveNurseZombies())
		{
			AC_NurseZombie* ActiveNurse = Cast<AC_NurseZombie>(Zombie);
			if (!ActiveNurse) continue;
			
			if (ActiveNurse->TryRegisterAsHealTarget(this))
			{
				++m_HealRequestRegisterCount; // 등록 횟수 하나 올리기
				break; // Nurse HealTarget에 정상 등록 처리됨 (Available한 Nurse가 없을 수도 있음)
			}
		}
	}
	
	return DamageAmount;
}

void AC_BasicEnemy::OnHPIncreased(AC_BasicCharacter* _HPIncreasedCharacter)
{
	// 서버 환경의 Enemy인 경우에만 호출처리됨
	
	// 이미 HealedEffect 재생중인 경우
	if (m_HealedEffectNGComponent->IsActive()) return;
	{
		m_HealedEffectNGComponent->Activate(true);
		Multicast_ToggleHealedEffect(true);
	}
}

void AC_BasicEnemy::Multicast_ToggleHealedEffect_Implementation(bool _Activate)
{
	// 서버 쪽은 이미 해당 처리를 한 상황
	if (IsLocallyControlled()) return;
	
	if (_Activate)
	{
		if (m_HealedEffectNGComponent->IsActive()) return;
			m_HealedEffectNGComponent->Activate(true);
	}
	else m_HealedEffectNGComponent->DeactivateImmediate();
}

void AC_BasicEnemy::DropItemOnDead()
{
	if (m_DropTableDataAsset && IsValid(m_DropTableDataAsset))
	{
		if (!m_ItemManager) return;
		
		const FVector DeathLocation = GetActorLocation();
		// 데이터 에셋의 드랍 항목들 순회
		for (const FDropEntry& Entry : m_DropTableDataAsset->DropEntries)
		{
			// 1. 드랍 확률 체크 (0.0f ~ 1.0f)
			const float RandValue = FMath::FRand(); 
			if (RandValue <= Entry.DropChance)
			{
				// 2. MinCount ~ MaxCount 사이의 랜덤 스폰 수량 결정
				const int32 SpawnCount = FMath::RandRange(Entry.MinCount, Entry.MaxCount);
				// 수량이 0 이하라면 스폰 스킵
				if (SpawnCount <= 0) 
					continue;
				// 3. 아이템이 약간 퍼져서 떨어지도록 원형 오프셋 계산
				const FVector2D RandCircle = FMath::RandPointInCircle(m_DropTableDataAsset->m_DropScatterRadius);
				const FVector SpawnLocation = DeathLocation + FVector(RandCircle.X, RandCircle.Y, 10.f);
				// 4. ItemManager를 통해 아이템 스폰
				m_ItemManager->SpawnItemPickUp(Entry.ItemRowName, SpawnCount, SpawnLocation);
			}
		}
	}
}

void AC_BasicEnemy::OnDead(AC_BasicCharacter* _DeadCharacter)
{
	// 서버 환경의 Enemy인 경우에만 호출처리됨
	
	// TODO : Dead에 필요한 처리가 더 필요하다면 여기서 이어서 처리해줄 것(ex 랙돌 처리 등)
	// 아마 죽은 뒤에 죽은 모션이나 랙돌 처리를 보여준 후, 몇 초 뒤에 Pool로 돌아가게끔 처리를 해줄 듯
	
	if (!HasAuthority())
		return;

	// 죽음 중복 실행 방지
	if (m_DeadRepData.bDead)
		return;

	// 전달받은 정보에서 죽은 캐릭터가 자기 자신인지 확인
	if (_DeadCharacter != this)
		return;

	// 서버에서 사용할 죽음 몽타주 인덱스 선택
	int32 SelectedDeadMontageIndex = INDEX_NONE;

	if (!m_DeadMontages.IsEmpty())
	{
		SelectedDeadMontageIndex = FMath::RandRange(0, m_DeadMontages.Num() - 1);
	}
	else
	{
		UC_Util::Print("!!Dead Montage Array is Empty!!", FColor::Red, 10.f);
	}

	// 서버에서 죽음 상태와 선택한 몽타주 인덱스를 m_DeadRepData에 기록
	m_DeadRepData.bDead = true;
	m_DeadRepData.DeadMontageIndex = SelectedDeadMontageIndex;
	
	// 죽은 곳에 아이템 드랍
	DropItemOnDead();
	
	if (IsValid(m_HealedEffectNGComponent))
	{
		m_HealedEffectNGComponent->DeactivateImmediate();
		Multicast_ToggleHealedEffect(false);
	}

	// 실행중이던 모든 행동(AI 정지, 스킬 및 이동..) 정지시키기
	StopAllActionsForDead();

	// RepNotify는 서버에서 자동 호출되지 않으므로 
	// 서버 화면에는 직접 죽음 시각 처리 적용
	ApplyDeadState(m_DeadRepData.DeadMontageIndex);
	
	// TODO : 죽은동안 충돌 끄기. 혹시 오브젝트 풀링으로 사용중이거나 해서 나중에 켜야 된다면 켜주어야 함.
	// 풀에서 꺼낼 때 복구하기
	//SetActorEnableCollision(false);

	// 변경한 복제 정보를 가능한 빨리 클라에 전달
	ForceNetUpdate();

	// 죽음 뒤 일정시간 후 ZombieManager의 풀 반환
	if (m_DeadRemainTime > 0.f)
	{
		// 서버의 OnDead에서만 타이머 생성
		// 클라는 풀 반환을 직접 계산하지 않고 
		// 서버가 복제한 풀 활성 상태를 따라가게
		GetWorldTimerManager().SetTimer(m_DeadRemainTimer, this, &AC_BasicEnemy::FinishDead, m_DeadRemainTime, false);
	}
	else
	{
		// 유지시간이 0이면 바로 풀로 반환
		FinishDead();
	}
}

void AC_BasicEnemy::StopAllActionsForDead()
{
	// 이동 및 비헤이비어트리 정지
	if (AAIController* pController = Cast<AAIController>(GetController()))
	{
		pController->StopMovement();

		// 비헤이비어트리에서 완전히 정지시키기
		if (UBrainComponent* Brain = pController->GetBrainComponent())
		{
			// 실행중인 useskill task가 abort 되면서
			// ontaskfinished에서 endskillmanully() 호출
			Brain->StopLogic(TEXT("Enemy Dead"));
		}
	}

	// 이동 즉시 정지 및 비활성화
	if (UCharacterMovementComponent* MoveCom = GetCharacterMovement())
	{
		MoveCom->StopMovementImmediately();
		MoveCom->DisableMovement();
	}

	// BT가 Abort된 이후에도 스킬 상태가 남은 상태만 직접 정지 처리
	if (IsValid(m_SkillCom) && m_SkillCom->IsUsingSkill())
	{
		m_SkillCom->EndSkillManually();
	}
	else
	{
		// 스킬을 사용중이지 않아도 다른 몽타주가 재생 중
		// 일수도 있어서 정지
		StopAnimMontage();
	}
}

void AC_BasicEnemy::ApplyDeadState(int32 _DeadMontageIndex)
{
	// 클라에 남아있던 공격 또는 스킬 몽타주 정지
	StopAnimMontage();

	// 서버가 선택한 죽음 몽타주 재생
	PlayDeadAnimation(_DeadMontageIndex);

	// 서버와 클라 동시 즉시 충돌 비활성화
	//SetActorEnableCollision(false);
}

void AC_BasicEnemy::PlayDeadAnimation(int32 _DeadMontageIndex)
{
	// 서버에서 전달한 인덱스가 배열 범위 안인지 검사
	if (!m_DeadMontages.IsValidIndex(_DeadMontageIndex))
		return;

	UAnimMontage* SelectedMontage = m_DeadMontages[_DeadMontageIndex];

	// 선택된 몽타주 유효성 검사
	if (!IsValid(SelectedMontage))
		return;

	// 서버와 클라에서 같은 죽음 몽타주 재생
	PlayAnimMontage(SelectedMontage);

	PlayDeadSound();
}

void AC_BasicEnemy::OnRep_DeadData()
{
	if (m_DeadRepData.bDead)
	{
		ApplyDeadState(m_DeadRepData.DeadMontageIndex);

		return;
	}

	// 풀에서 다시 활성화 된 경우
	StopAnimMontage();
	
	// 클라에서도 죽은 좀비의 Collision 꺼주기.
	//SetActorEnableCollision(false);
}

void AC_BasicEnemy::FinishDead()
{
	// 서버에서만 Zombie Pool 반환 처리
	if (!HasAuthority())
		return;

	// 중복 타이머 실행방지 및 핸들 정리
	GetWorldTimerManager().ClearTimer(m_DeadRemainTimer);

	// C_Zombie 를 상속받은 계열만 ZombieManager Pool로 반환
	AC_Zombie* Zombie = Cast<AC_Zombie>(this);

	if (!IsValid(Zombie))
	{
		UC_Util::Print("From AC_BasicEnemy::FinishDead : This Enemy is not Zombie", FColor::Red, 10.f);

		return;
	}

	UC_ZombieManager* ZombieManager = ZOMBIE_MANAGER(this);

	// 죽음 몽타주가 종료된 Zombie를 대기 Pool로 반환
	if (!IsValid(ZombieManager))
	{
		UC_Util::Print("From AC_BasicEnemy::FinishDead : ZombieManager is nullptr !!", FColor::Red, 10.f);

		return;
	}

	// 실제 비활성화와 풀 등록은 ZombieManager에 전달해서
	// ZombieManager 가 처리
	if (!ZombieManager->ReturnZombieToPool(Zombie))
	{
		UC_Util::Print("From AC_BasicEnemy::FinishDead : ReturnZombieToPool Failed !!", FColor::Red, 10.f);
	}
}


void AC_BasicEnemy::DecreaseHealRequestRegisterCount()
{
	if (--m_HealRequestRegisterCount < 0)
	{
		UC_Util::Print("From AC_BasicEnemy::DecreaseHealRequestRegisterCount : Wrong HealRequestRegisterCount decrease executed", FColor::Red, 10.f);
		m_HealRequestRegisterCount = 0;
	}
}

void AC_BasicEnemy::PlayCurrentSkillHitSound(const FVector& _Location)
{
	if (!IsValid(m_SkillCom))
		return;

	const TSoftObjectPtr<UC_EnemySkillData> CurSkillData = m_SkillCom->GetCurSkillData();
	
	if (!CurSkillData.IsValid())
		return;

	PlaySkillHitSound(CurSkillData.Get(), _Location);
}

void AC_BasicEnemy::PlaySkillHitSound(UC_EnemySkillData* _SkillData, const FVector& _Location)
{
	// 서버에서만 multicast 요청
	if (!HasAuthority())
		return;

	if (!IsValid(_SkillData))
		return;

	if (!IsValid(_SkillData->HitSound))
		return;

	Multicast_PlaySkillHitSound(_SkillData->HitSound, _Location);
}

void AC_BasicEnemy::Multicast_PlaySkillHitSound_Implementation(USoundBase* _Sound, FVector _Location)
{
	if (!IsValid(_Sound))
		return;

	UGameplayStatics::PlaySoundAtLocation(this, _Sound, _Location);
}
