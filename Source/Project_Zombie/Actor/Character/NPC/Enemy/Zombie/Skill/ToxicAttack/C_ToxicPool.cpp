// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ToxicPool.h"

#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"

#include "Actor/Character/Player/C_BasicPlayer.h"

#include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"
#include "Actor/Character/NPC/Enemy/Zombie/Skill/C_EnemySkillData.h"

#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "Utility/C_Util.h"

AC_ToxicPool::AC_ToxicPool()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	// 1. 충돌 컴포넌트
	m_Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	RootComponent = m_Sphere;
	m_Sphere->InitSphereRadius(100.f);
	// SetCollisionProfileName(TEXT("Projectile"));

	// 충돌 설정
	// Pawn만 오버랩 처리
	m_Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	m_Sphere->SetCollisionObjectType(ECC_WorldDynamic);
	m_Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	m_Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 오버랩 이벤트 활성화
	m_Sphere->SetGenerateOverlapEvents(true);

	// 2. NiagaraComponent
	m_NiagaraCom = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	m_NiagaraCom->SetupAttachment(m_Sphere); // 자식으로 설정

	m_Sphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&AC_ToxicPool::OnBeginOverlap);

	m_Sphere->OnComponentEndOverlap.AddDynamic(
		this,
		&AC_ToxicPool::EndOverlap);
}

void AC_ToxicPool::BeginPlay()
{
	Super::BeginPlay();

	if (m_PoolEffect)
	{
		m_NiagaraCom->SetAsset(m_PoolEffect);
		m_NiagaraCom->Activate();
	}

}

void AC_ToxicPool::InitPool(AC_BasicEnemy* _SkillUser, UC_EnemySkillData* _Skill)
{
	//UC_Util::Print("InitPool");

	m_SkillUser = _SkillUser;
	m_Skill = _Skill;

	SetLifeSpan(m_PoolLifeTime);

	if (HasAuthority())
	{
		// ApplyDamage를 데미지 간격마다 실행시킨다
		GetWorldTimerManager().SetTimer(
										m_DamageTimer,
										this,
										&AC_ToxicPool::ApplyTickDamage,
										m_DamageInterval,
										true);
	}
}

void AC_ToxicPool::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버인지 체크
	if (!HasAuthority())
		return;

	// 유효한 액터인지 확인
	if (!IsValid(OtherActor))
		return;

	// 본인과의 충돌을 무시
	if (OtherActor == this)
		return;

	// 장판을 생성한 좀비라면 장판 데미지 무시
	if (OtherActor == m_SkillUser)
		return;

	// 플레이어인지 확인
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor);
	if (!Player)
		return;

	// 데미지를 받을 타겟 등록
	m_OverlapTargets.Add(OtherActor);
}

void AC_ToxicPool::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 서버에서만 목록관리
	if (!HasAuthority())
		return;

	// 유효한 액터인지 확인
	if (!IsValid(OtherActor))
		return;

	// 플레이어인지 확인
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(OtherActor);
	if (!Player)
		return;

	// 장판 밖으로 나간 플레이어는 목록에서 제거
	m_OverlapTargets.Remove(OtherActor);
}

void AC_ToxicPool::ApplyTickDamage()
{
	UE_LOG(LogTemp, Warning, TEXT("ToxicPool Timer Tick / TargetNum: %d"), m_OverlapTargets.Num());

	// 서버인지 확인
	if (!HasAuthority())
		return;

	// 스킬이 유효한지 확인
	if (!IsValid(m_Skill))
		return;

	// 등록된 플레이어가 없으면 return
	if (m_OverlapTargets.Num() == 0)
		return;

	AController* InstigatorController = nullptr;

	// 장판을 생성한 좀비의 컨트롤러를 가져온다
	if (IsValid(m_SkillUser))
	{
		InstigatorController = m_SkillUser->GetController();
	}

	// 장판안에 등록된 대상 순회
	for (auto Iter = m_OverlapTargets.CreateIterator(); Iter; ++Iter)
	{
		// 검사중인 플레이어 가져오기
		AActor* Target = Iter->Get();

		// 플레이어가 유효한지 확인
		if (!IsValid(Target))
		{
			// 목록에서 제거
			Iter.RemoveCurrent();
			continue;
		}

		// 데미지를 발생시킨 개체 가져오기 (m_SkillUser가 nullptr 이라면 this(장판) 가져오기)
		AActor* DamageCauser = IsValid(m_SkillUser) ? static_cast<AActor*>(m_SkillUser) : this;

		// 데미지 적용
		UGameplayStatics::ApplyDamage(Target, m_Skill->Damage, InstigatorController, DamageCauser, UDamageType::StaticClass());
	}
}

