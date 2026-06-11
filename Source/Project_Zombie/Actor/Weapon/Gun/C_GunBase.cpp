// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GunBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "TimerManager.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

AC_GunBase::AC_GunBase()
{
	PrimaryActorTick.bCanEverTick = true;

	m_WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = m_WeaponMesh;
}

void AC_GunBase::BeginPlay()
{
	Super::BeginPlay();
	
	m_CurrentAmmo = m_MaxAmmo;
}

void AC_GunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AC_GunBase::PullTrigger()
{
	if (m_bIsFiring) return; // 이미 쏘고 있다면 중복 실행 방지
	m_bIsFiring = true;

	// 누르자마자 딜레이 없이 즉시 한 발 발사
	PlayFireEffects();

	// m_FireRate(연사 속도) 간격으로 PlayFireEffects 함수를 무한 반복 호출
	// 마지막 인자인 true가 반복
	if (m_bIsFiring)
	{
		GetWorldTimerManager().SetTimer(m_FireTimerHandle, this, &AC_GunBase::PlayFireEffects, m_FireRate, true);
	}
}

void AC_GunBase::ReleaseTrigger()
{
	if (!m_bIsFiring) return;
	m_bIsFiring = false;

	// 작동 중이던 연사 타이머 중지
	GetWorldTimerManager().ClearTimer(m_FireTimerHandle);
}

void AC_GunBase::Reload()
{
	ReleaseTrigger();

	if (m_CurrentAmmo == m_MaxAmmo) return;

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Reloading..."));

	// 2초 타이머 후 탄창만큼의 탄약 보충
	FTimerHandle ReloadTimerHandle;
	GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AC_GunBase::CompleteReload, 2.0f, false);
}

void AC_GunBase::CompleteReload()
{
	m_CurrentAmmo = m_MaxAmmo;
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Reload Complete"));
}

// 총알 소모 로직 후 애니메이션 실행 함수
void AC_GunBase::PlayFireEffects()
{
	// 총알이 없다면 사격 중지
	if (m_CurrentAmmo <= 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("No Ammo! Need Reload (Press R)"));
		ReleaseTrigger();
		return;
	}

	m_CurrentAmmo--;

	FString AmmoLog = FString::Printf(TEXT("Ammo: %d / %d"), m_CurrentAmmo, m_MaxAmmo);
	GEngine->AddOnScreenDebugMessage(-1, m_FireRate, FColor::Green, AmmoLog);

	// 총기 발사 애니메이션 재생
	if (m_WeaponMesh && m_FireAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_FireAnimation, false);
	}

	// 탄피 배출 로직 시작
	if (m_ShellMesh && m_WeaponMesh && GetWorld())
	{
		// 총기 메쉬에 생성해 둔 탄피 배출구 소켓("AmmoEject")의 월드 좌표 및 회전값 가져오기
		FTransform EjectTransform = m_WeaponMesh->GetSocketTransform(TEXT("AmmoEject"), RTS_World);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 탄피용 기본 액터를 월드에 생성
		AActor* SpawnedShell = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), EjectTransform, SpawnParams);

		if (SpawnedShell)
		{
			// 동적으로 스태틱 메시 컴포넌트 생성
			UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(SpawnedShell, TEXT("ShellMeshComp"));
			if (MeshComp)
			{
				// 메시 에셋 지정 및 모빌리티를 Movable로 변경
				MeshComp->SetStaticMesh(m_ShellMesh);
				MeshComp->SetMobility(EComponentMobility::Movable);

				// 컴포넌트를 액터의 루트(최상단)로 등록하여 조립 완료
				SpawnedShell->SetRootComponent(MeshComp);
				MeshComp->RegisterComponent();

				// 컴포넌트가 등록된 액터의 위치를 총기 소켓 좌표로 이동
				SpawnedShell->SetActorTransform(EjectTransform);

				// 물리 엔진 및 콜리전(PhysicsActor) 활성화
				MeshComp->SetSimulatePhysics(true);
				MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));

				// 탄피 배출 힘(Impulse) 계산 및 적용
				// 소켓의 현재 회전 기준 우측(Right) 벡터와 상단(Up) 벡터를 활용해 항상 총구 기준 우측 상단으로 튕김
				FVector EjectDirection = EjectTransform.GetRotation().GetRightVector() * 150.0f
					+ EjectTransform.GetRotation().GetUpVector() * 75.0f;

				MeshComp->AddImpulse(EjectDirection, NAME_None, true);

				// 3초 뒤 월드에서 자동으로 파괴되도록 수명 설정
				SpawnedShell->SetLifeSpan(3.0f);
			}
		}
	}
}