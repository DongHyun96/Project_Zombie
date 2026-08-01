// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkillComponent/C_EnemySkillComponent.h"
#include "Components/StatComponent/C_EnemyStatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameModeAndManager/C_GameMode_GameLv.h"
#include "BrainComponent.h"
#include "GameModeAndManager/C_ZombieManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/C_Util.h"
#include "Zombie/NurseZombie/C_NurseZombie.h"
#include "Zombie/Controller/C_ZombieController.h"

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
	
}

void AC_BasicEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		// 죽었을 때 처리할 함수 Delegate 구독 처리 (죽은 이후로도 다음에 Spawn 처리가 될 수 있기 때문에 구독 해지는 처리하지 않는다)
		m_StatComponent->OnCurHPReachedZeroDelegate.AddUObject(this, &AC_BasicEnemy::OnDead);

		// IncreaseCurHP 정상 처리 시(힐 받은 처리로 판단) -> 힐 받은 Effect 활성화 함수 Delegate 구독 처리
		m_StatComponent->OnIncreaseCurHPDelegate.AddUObject(this, &AC_BasicEnemy::OnHPIncreased);
		
		m_ZombieController = GetController<AC_ZombieController>();
	}
	else // 클라이언트 환경
	{
		// 클라이언트단 화면에서는 Controller가 없기에, Controller Rotation (0, 0, 0) 값을 사용ㄴ
		// 따라서 끊겨보이는 버그가 있었음
		bUseControllerRotationYaw                             = false;
		GetCharacterMovement()->bOrientRotationToMovement     = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate                  = FRotator(0.f, 360.f, 0.f);
		
	}

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
	
	// UC_Util::Print("Zombie Damaged", FColor::Red, 10.f);

	/* 힐 요청 처리 관련 */

	// 이미 힐 요청 최대 등록 횟수를 기록
	if (m_HealRequestRegisterCount >= s_MaxHealRequestRegisterCount) return DamageAmount;
	
	// 현재 생명력 Ratio 50% ~ 70% 랜덤 수치 이하면, 가능한 힐러 좀비에게 힐 요청 시도
	// TODO : 이거 요청 빈도가 너무 높으면 여기서 병목 생길수도 있음 -> 추후 최적화할 때 고려할 것
	if (m_StatComponent->GetCurHPRatio() < FMath::RandRange(0.5f, 0.7f))
	{
		for (AC_NurseZombie* ActiveNurse : ZOMBIE_MANAGER(this)->GetActiveNurseZombies())
			if (ActiveNurse->TryRegisterAsHealTarget(this))
			{
				++m_HealRequestRegisterCount; // 등록 횟수 하나 올리기
				break; // Nurse HealTarget에 정상 등록 처리됨 (Available한 Nurse가 없을 수도 있음)
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

void AC_BasicEnemy::OnDead(AC_BasicCharacter* _DeadCharacter)
{
	// 서버 환경의 Enemy인 경우에만 호출처리됨
	

	// TODO : Dead에 필요한 처리가 더 필요하다면 여기서 이어서 처리해줄 것(ex 랙돌 처리 등)
	// 아마 죽은 뒤에 죽은 모션이나 랙돌 처리를 보여준 후, 몇 초 뒤에 Pool로 돌아가게끔 처리를 해줄 듯
	
	if (!HasAuthority())
		return;

	if (m_bDead)
		return;

	if (_DeadCharacter != this)
		return;

	m_bDead = true;

	if (IsValid(m_HealedEffectNGComponent))
	{
		m_HealedEffectNGComponent->DeactivateImmediate();
		Multicast_ToggleHealedEffect(false);
	}

	// 실행중이던 모든 행동(AI 정지, 스킬 및 이동..) 정지시키기
	StopAllActionsForDead();

	// 죽음 애니메이션 재생
	PlayDeadAnimation();
}

void AC_BasicEnemy::StopAllActionsForDead()
{
	if (AAIController* pController = Cast<AAIController>(GetController()))
	{
		pController->StopMovement();

		// 비헤이비어트리에서 완전히 정지시키기
		if (UBrainComponent* Brain = pController->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Enemy Dead"));
		}
	}

	if (UCharacterMovementComponent* MoveCom = GetCharacterMovement())
	{
		MoveCom->StopMovementImmediately();
		MoveCom->DisableMovement();
	}

	// 재생 중이던 스킬 몽타주 정지
	StopAnimMontage();

	// 현재 사용중인 스킬 강제 종료
	if (IsValid(m_SkillCom))
	{
		// 스킬컴포넌트에서 스킬 종료 함수가 있다면 호출
	}
}

void AC_BasicEnemy::PlayDeadAnimation()
{
	// 여러개의 죽음 몽타주 중 랜덤하게 play
	if (m_DeadMontages.IsEmpty())
	{
		UC_Util::Print("!!Dead Montage is Empty!!", FColor::Red, 10.f);
		return;
	}

	const int32 RandomIndex = FMath::RandRange(0, m_DeadMontages.Num() - 1);

	UAnimMontage* SelectedMontage = m_DeadMontages[RandomIndex];

	if (!IsValid(SelectedMontage))
	{
		UC_Util::Print("!!Selected Dead Montage is nullptr!!", FColor::Red, 10.f);
		return;
	}

	PlayAnimMontage(SelectedMontage);
}

/* 동기화 처리 후 마무리
void AC_BasicEnemy::FinishDead()
{
	// 서버에서만 Zombie Pool 반환 처리
	if (!HasAuthority())
		return;

	// C_Zombie 를 상속받은 계열만 ZombieManager Pool로 반환
	AC_Zombie* Zombie = Cast<AC_Zombie>(this);

	if (!IsValid(Zombie))
	{
		UC_Util::Print("From AC_BasicEnemy::FinishDead : This Enemy is not Zombie", FColor::Red, 10.f);

		return;
	}

	// 죽음 몽타주가 종료된 Zombie를 대기 Pool로 반환
	if (!ZOMBIE_MANAGER->ReturnZombieToPool(Zombie))
	{
		UC_Util::Print("From AC_BasicEnemy::FinishDead : ReturnZombieToPool Failed !!", FColor::Red, 10.f);

	}
}*/


void AC_BasicEnemy::DecreaseHealRequestRegisterCount()
{
	if (--m_HealRequestRegisterCount < 0)
	{
		UC_Util::Print("From AC_BasicEnemy::DecreaseHealRequestRegisterCount : Wrong HealRequestRegisterCount decrease executed", FColor::Red, 10.f);
		m_HealRequestRegisterCount = 0;
	}
}
