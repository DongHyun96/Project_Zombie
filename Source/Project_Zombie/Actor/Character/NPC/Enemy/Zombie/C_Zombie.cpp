// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Zombie.h"


#include "../../../GlobalEnum.h"
#include "Actor/Character/NPC/Enemy/Components/SkillComponent/C_EnemySkillComponent.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "Actor/PointTower/C_PointTower.h"
#include "Net/UnrealNetwork.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Components/ShapeComponent.h"
#include "Sound/SoundBase.h"
#include "Controller/C_ZombieController.h"
#include "GameModeAndManager/C_GameMode_GameLv.h"
#include "GameModeAndManager/C_ZombieManager.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Utility/C_Util.h"

AC_Zombie::AC_Zombie()
	: AC_BasicEnemy()
	, m_ZombieType(EZombieType::NormalZombie)
{
	m_TeamId = static_cast<uint8>(ETeamType::Enemy);
}

AC_Zombie::AC_Zombie(EZombieType _ZombieType)
	: AC_BasicEnemy()
	, m_ZombieType(_ZombieType)
{
	m_TeamId = static_cast<uint8>(ETeamType::Enemy);

	m_Sound = CreateDefaultSubobject<UAudioComponent>(TEXT("Sound"));

	m_Sound->SetupAttachment(GetRootComponent());

	m_Sound->bAutoActivate = false;
}

void AC_Zombie::BeginPlay()
{
	Super::BeginPlay();
	
	// 등록된 모든 NormalAttackCollider의 ComponentBeginOverlap 이벤트 바인딩 및 첫 시작 시, 비활성화 처리
	for (UShapeComponent* NormalAttackCollider : m_NormalAttackColliders)
	{
		// 서버 쪽에서만 실질적인 피격 판정 및 피격 처리가 들어갈 것임
		if (HasAuthority())
			NormalAttackCollider->OnComponentBeginOverlap.AddDynamic(this, &AC_Zombie::OnNormalAttackColliderBeginOverlap);
		
		NormalAttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// 레벨에 사용자가 직접 놓은 좀비의 경우, ZombieManager의 ActiveZombies에 직접 등록시켜준다. -> 주로 테스팅용 좀비로 Level에 직접 배치한 좀비인 경우 이 경우에 해당
	if (HasAuthority() && m_bIsLevelPlaced)
	{
		if (UC_ZombieManager* ZombieManager = ZOMBIE_MANAGER(this))
			ZombieManager->AddZombieToActivePoolManually(m_ZombieType, this);
		else UC_Util::Print("[AC_Zombie::BeginPlay] : ZombieManager nullptr", FColor::Red, 10.f);
	}
}

void AC_Zombie::ANS_OnNormalAttackStart()
{
	// 오로지 서버 쪽에서 피격 판정 및 데미지 주기 처리
	if (!HasAuthority()) return;

	// 피격판정 시작 전, Set 비우기
	m_SetNormalAttackColliderEntered.Empty();
	
	// 피격 검사 활성화
	for (UShapeComponent* AttackCollider : m_NormalAttackColliders)
		AttackCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AC_Zombie::ANS_OnNormalAttackEnd()
{
	if (!HasAuthority()) return;

	m_SetNormalAttackColliderEntered.Empty();

	for (UShapeComponent* AttackCollider : m_NormalAttackColliders)
		AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AC_Zombie::OnNormalAttackColliderBeginOverlap
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
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor);
	AC_PointTower* PointTower = Cast<AC_PointTower>(OtherActor);
	if (!Player && !PointTower) return; // PointTower나 Player가 아닌 경우
	
	// 이미 이번 휘두르기에 피격처리가 한 번 들어감
	if (m_SetNormalAttackColliderEntered.Contains(OtherActor)) return;
	
	m_SetNormalAttackColliderEntered.Add(OtherActor);

	// 현재 Skill의 피격량을 구해와서, 대상 Target에게 ApplyDamage 처리
	const float AppliedDamage = UGameplayStatics::ApplyDamage
	(
		OtherActor,
		m_SkillCom->GetCurSkillDamage(),
		GetController(),
		this,
		nullptr
	);

	// 플레이어가 맞았을때 skilldata의 hitsound 재생
	if (Player && AppliedDamage > 0.f)
	{
		PlayCurrentSkillHitSound(OtherActor->GetActorLocation());
	}
}

void AC_Zombie::ApplyPoolActiveState()
{
	if (m_bPoolActive)
	{
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		SetActorTickEnabled(true);

		// 이동 활성화
		if (UCharacterMovementComponent* MoveCom = GetCharacterMovement())
		{
			MoveCom->StopMovementImmediately();
			MoveCom->SetMovementMode(EMovementMode::MOVE_Walking);
		}
		
		return;
	}

	// =====================풀 비활성화 상태=========================

	// AI 정지
	if (AC_ZombieController* pController = Cast<AC_ZombieController>(GetController()))
	{
		// 이전 이동 요청 제거
		pController->StopMovement();

		if (UBrainComponent* Brain = pController->GetBrainComponent())
			Brain->StopLogic(TEXT("Zombie in Pool"));
		
		// 인지 기능 비활성화
		pController->GetAIPerceptionComponent()->ForgetAll();
		pController->GetAIPerceptionComponent()->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
		pController->GetAIPerceptionComponent()->SetSenseEnabled(UAISense_Damage::StaticClass(), false);
		
		// ZombieController에서 기록한 인식된 Target 후보군 Actor 정보들 비우기
		pController->ClearAllSensedTarget();
	}
	
	// 풀 반환 시 남아있는 몽타주 정지
	StopAnimMontage();

	// 이동 정지
	if (UCharacterMovementComponent* MoveCom = GetCharacterMovement())
	{
		MoveCom->StopMovementImmediately();
		MoveCom->DisableMovement();
	}

	// 충돌 비활성화
	SetActorEnableCollision(false);

	// 화면 숨김처리
	SetActorHiddenInGame(true);

	// tick 비활성화
	SetActorTickEnabled(false);
	
}

void AC_Zombie::OnRep_PoolActive()
{
	// 클라이언트도 서버랑 같은 상태 적용
	ApplyPoolActiveState();
}

void AC_Zombie::OnDead(AC_BasicCharacter* _DeadCharacter)
{
	StopChaseSoundLoop();

	Super::OnDead(_DeadCharacter);
}

bool AC_Zombie::DeactivateForPool()
{
	// 풀 상태 변경은 서버에서만 처리
	if (!HasAuthority())
		return false;

	// 중복 처리 방지
	if (!m_bPoolActive)
		return false;

	// 풀로 들어가기전에 추격음 정지
	StopChaseSoundLoop();

	m_bPoolActive = false;

	// 서버에 바로 비활성 상태 적용
	ApplyPoolActiveState();

	// 클라에 최대한 빨리 전달
	ForceNetUpdate();

	return true;
}

bool AC_Zombie::ActivateFromPool(const FTransform& _SpawnTransform)
{
	// 풀 활성 상태는 서버에서만 처리
	if (!HasAuthority())
		return false;

	// 이미 활성화 되어잇는 좀비는 중복처리 x
	if (m_bPoolActive)
		return false;

	// 스폰 위치 먼저 설정
	// 숨김 상태가 풀리기 전에 먼저 옮겨두려고 미리 설정
	SetActorTransform(_SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);

	// 좀비 공통 상태 초기화
	ResetEnemyForPoolSpawn();

	// 풀 활성 상태로 변경
	m_bPoolActive = true;

	// 서버에 활성 상태 적용
	ApplyPoolActiveState();

	// 죽은 이후 StopLogic으로 정지된 BT 다시 시작
	if (AAIController* pController = Cast<AAIController>(GetController()))
	{
		// 이전 이동 요청 제거
		pController->StopMovement();

		if (UBrainComponent* Brain = pController->GetBrainComponent())
			Brain->RestartLogic();
		
		// 인지 기능 활성화
		pController->GetAIPerceptionComponent()->SetSenseEnabled(UAISense_Sight::StaticClass(), true);
		pController->GetAIPerceptionComponent()->SetSenseEnabled(UAISense_Damage::StaticClass(), true);
	}

	// 클라에 상태와 위치를 빠르게 전달
	ForceNetUpdate();

	return true;
}

void AC_Zombie::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 풀 활성 여부를 클라에 복제
	DOREPLIFETIME(AC_Zombie, m_bPoolActive);
}

void AC_Zombie::PlayRandomVoice(const TArray<TObjectPtr<USoundBase>>& _Sounds)
{
	// AudioComponent가 없으면 재생 불가
	if (!IsValid(m_Sound))
		return;

	// 등록된 사운드가 없으면 재생하지 않음
	if (_Sounds.IsEmpty())
		return;

	// 랜덤 인덱스 선택
	const int32 RandomIndex =
		FMath::RandRange(0, _Sounds.Num() - 1);

	USoundBase* Sound = _Sounds[RandomIndex];

	if (!IsValid(Sound))
		return;

	// 기존 음성 중단
	if (m_Sound->IsPlaying())
	{
		m_Sound->Stop();
	}

	// 새로운 사운드 설정 후 재생
	m_Sound->SetSound(Sound);
	m_Sound->Play();
}

void AC_Zombie::PlayHitSound()
{
	PlayRandomVoice(m_HitSounds);
}

void AC_Zombie::PlayDeadSound()
{
	PlayRandomVoice(m_DeadSounds);
}

void AC_Zombie::PlayIdleSound()
{
	PlayRandomVoice(m_IdleSounds);
}

void AC_Zombie::PlayChaseSound()
{
	PlayRandomVoice(m_ChaseSounds);
}

void AC_Zombie::ScheduleNextChaseSound()
{
	if (!HasAuthority())
		return; 

	if (!m_bChaseSoundLoopActive)
		return;

	const float RnadomDelay = FMath::FRandRange(m_ChaseSoundMinInterval, m_ChaseSoundMaxInterval);

	GetWorldTimerManager().SetTimer(m_ChaseSoundTimer, this, &AC_Zombie::OnChaseSoundTimer, RnadomDelay, false);
}

void AC_Zombie::OnChaseSoundTimer()
{
	if (!HasAuthority())
		return;

	if (!m_bChaseSoundLoopActive)
		return;

	// 추격음 재생
	Multicast_PlayChaseSound();

	// 다시 다음 랜덤시간 예약
	ScheduleNextChaseSound();
}

void AC_Zombie::StartChaseSoundLoop()
{
	// AI 및 Timer는 서버에서만 관리
	if (!HasAuthority())
		return;

	// 이미 실행 중이면 또 시작하지 않음
	if (m_bChaseSoundLoopActive)
		return;

	m_bChaseSoundLoopActive = true;

	// 추격을 처음 시작할 때 바로 한 번 재생
	Multicast_PlayChaseSound();

	// 다음 추격음 예약
	ScheduleNextChaseSound();
}

void AC_Zombie::StopChaseSoundLoop()
{
	if (!HasAuthority())
		return;

	m_bChaseSoundLoopActive = false;

	GetWorldTimerManager().ClearTimer(m_ChaseSoundTimer);
}

void AC_Zombie::Multicast_PlayChaseSound_Implementation()
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[ChaseSound Multicast] Zombie=%s / Role=%d / NetMode=%d"),
		*GetName(),
		static_cast<int32>(GetLocalRole()),
		static_cast<int32>(GetNetMode())
	);

	PlayChaseSound();
}

void AC_Zombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
