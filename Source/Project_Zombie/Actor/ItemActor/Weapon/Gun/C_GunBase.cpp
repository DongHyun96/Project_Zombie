// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GunBase.h"
#include "Animation/AnimSequence.h"
#include "TimerManager.h"
#include "Engine/StaticMeshActor.h"
#include "DrawDebugHelpers.h"
#include "Actor/Character/Player/C_BasicPlayer.h"
#include "../WeaponComponent/GunComponent/C_GunDataTableComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

#include "GameMode/C_UIManager.h"
#include "UI/MainHUD/C_GameMainHUD.h"
#include "Utility/C_Util.h"

// 일단은 총기 오른손 부착 위치 Socket과 동일한 Socket으로 둠
const FName AC_GunBase::s_HandSocketName = TEXT("HandGrip_R");

AC_GunBase::AC_GunBase()
{
	PrimaryActorTick.bCanEverTick = true;

	m_WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = m_WeaponMesh;

	m_DataCom = CreateDefaultSubobject<UC_GunDataTableComponent>(TEXT("DataComponent"));
}

void AC_GunBase::BeginPlay()
{
	Super::BeginPlay();
	
	m_CurrentAmmo = m_MaxAmmo;
}

/*void AC_GunBase::StartAttack()
{
	PullTrigger();
}

void AC_GunBase::StopAttack()
{
	ReleaseTrigger();
}

void AC_GunBase::Reload()
{
	Gun_Reload();
}*/

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

void AC_GunBase::Gun_Reload()
{
	ReleaseTrigger();

	if (m_CurrentAmmo == m_MaxAmmo) 
		return;

	// 재장전 애니메이션 재생
	if (m_WeaponMesh && m_ReloadAnimation)
	{
		m_WeaponMesh->PlayAnimation(m_ReloadAnimation, false);
	}

	// 2초 타이머 후 탄창만큼의 탄약 보충
	FTimerHandle ReloadTimerHandle;
	GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AC_GunBase::CompleteReload, 2.0f, false);
}

bool AC_GunBase::AttachToHand(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false

	// Main HUD MeleeWeapon 종류로 초기화
	if (APlayerController* PC = Player->GetController<APlayerController>())
	{
		// TODO : 각 MeleeWeapon에 맞는 이미지 아이콘(?) 표시해주면 좋을 듯 (일단은 AmmoInfo쪽 정보 감추는 처리로 함)
		if (AC_UIManager* UIManager = Cast<AC_UIManager>(PC->GetHUD()))
			UIManager->GetMainHUDWidget()->ToggleAmmoInfoVisibility(true, EFireMode::FullAuto, m_CurrentAmmo, m_MaxAmmo); // TODO : FireMode 현재 FireMode로 넣어줄 것
	}

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		s_HandSocketName
	);
	
	if (bIsAttached)
		Player->SetHandState(EHandState::WeaponGun);
	
	return bIsAttached;
}

bool AC_GunBase::AttachToHolster(USceneComponent* _ParentMesh)
{
	if (!_ParentMesh) return false;
	AC_BasicPlayer* Player = Cast<AC_BasicPlayer>(_ParentMesh->GetOwner());
	if (!Player) return false; // 장착 시도하는 Owner Character가 Player형이 아닌 경우, return false

	const bool bIsAttached = AttachToComponent
	(
		_ParentMesh,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		m_HolsterSocketName
	);
	
	return bIsAttached;
}

void AC_GunBase::CompleteReload()
{
	m_CurrentAmmo = m_MaxAmmo;
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Reload Complete"));

	// 새로 장전된 장탄수 UI 업데이트
	if (AC_UIManager* UIManager = Cast<AC_UIManager>(GetWorld()->GetFirstPlayerController()->GetHUD()))
		UIManager->GetMainHUDWidget()->UpdateMagazineAmmoCount(m_CurrentAmmo);
}

// 총알 소모 로직 후 애니메이션 실행 함수
void AC_GunBase::PlayFireEffects()
{
	// 총알이 없다면 사격 중지
	if (m_CurrentAmmo <= 0)
	{
		if (AC_UIManager* UIManager = Cast<AC_UIManager>(GetWorld()->GetFirstPlayerController()->GetHUD()))
			UIManager->GetMainHUDWidget()->AddPlayerWarningLog("OUT OF AMMO");
		
		ReleaseTrigger();
		return;
	}

	m_CurrentAmmo--;

	// 현재 남은 장탄수 UI 업데이트
	if (AC_UIManager* UIManager = Cast<AC_UIManager>(GetWorld()->GetFirstPlayerController()->GetHUD()))
		UIManager->GetMainHUDWidget()->UpdateMagazineAmmoCount(m_CurrentAmmo);

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
				
				MeshComp->SetCollisionProfileName(TEXT("Custom"));

				// 캐릭터와 무기 컴포넌트는 완전히 충돌무시(Ignore)하도록 설정합니다.
				MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);        // 캐릭터 몸통 무시
				MeshComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore); // 총기 및 기타 물리 물리 무시

				// 바닥이나 벽에는 부딪혀야 하므로 Block으로 설정
				MeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);  // 벽, 땅 바닥
				MeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block); // 움직이는 장애물

				/* 제미나이 도움받음*******/

				float RandomRightForce = FMath::FRandRange(130.0f, 220.0f); // 오른쪽으로 튕기는 힘 (최소 130 ~ 최대 220)
				float RandomUpForce = FMath::FRandRange(60.0f, 130.0f);  // 위로 솟구치는 힘   (최소 60 ~ 최대 130)
				float RandomForwardForce = FMath::FRandRange(-40.0f, 40.0f); // 앞뒤로 미세하게 흔들리는 힘

				// 소켓의 방향 벡터에 랜덤 힘을 곱해서 최종 방향 벡터(Velocity) 생성
				FVector EjectDirection = (EjectTransform.GetRotation().GetRightVector() * RandomRightForce)
					+ (EjectTransform.GetRotation().GetUpVector() * RandomUpForce)
					+ (EjectTransform.GetRotation().GetForwardVector() * RandomForwardForce);

				// 탄피에 불규칙한 추진력 가하기
				MeshComp->AddImpulse(EjectDirection, NAME_None, true);

				// 날아갈 때 탄피 자체도 지멋대로 빙글빙글 회전하게 만들기 (Angular Impulse)
				// X, Y, Z축 기준으로 랜덤한 회전력을 줍니다.
				FVector RandomTorque = FVector(FMath::FRandRange(-50.0f, 50.0f),
					FMath::FRandRange(-50.0f, 50.0f),
					FMath::FRandRange(-50.0f, 50.0f));
				MeshComp->AddAngularImpulseInRadians(RandomTorque, NAME_None, true);

				/*******제미나이 도움받음 */

				// 3초 뒤 월드에서 자동으로 파괴되도록 수명 설정
				SpawnedShell->SetLifeSpan(3.0f);
			}
		}
	}
	if (m_WeaponMesh && GetWorld())
	{
		// 소켓 이름 : MuzzleFlash
		FVector StartLocation = m_WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
		FVector ForwardDirection = m_WeaponMesh->GetSocketRotation(TEXT("MuzzleFlash")).Vector();
		FVector MaxEndLocation = StartLocation + (ForwardDirection * 500.0f); // 최대 사거리

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this); // 총 액터 충돌 무시
		if (GetOwner())
		{
			QueryParams.AddIgnoredActor(GetOwner()); // 총을 들고있는 캐릭터 액터 충돌 무시
		}

		bool bHasHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			StartLocation,
			MaxEndLocation,
			ECC_Visibility,
			QueryParams
		);

		// 라인트레이스 디버그 모드 사거리 표시
		FVector ActualEndLocation;

		if (bHasHit)
		{
			ActualEndLocation = HitResult.ImpactPoint; // 충돌이 감지되면 출돌체 까지가 끝점
		}
		else
		{
			ActualEndLocation = MaxEndLocation; // 원래 끝 사거리
		}

		// 총알 궤적
		DrawDebugLine(
			GetWorld(),
			StartLocation,
			ActualEndLocation,
			FColor::Green,
			false,
			0.5f,
			0,
			1.5f
		);

		// 충돌체에 맞으면 붉은원으로 표시
		if (bHasHit)
		{
			DrawDebugSphere(
				GetWorld(),
				ActualEndLocation,
				7.0f,
				12,
				FColor::Red,
				false,
				0.5f,
				0,
				1.5f
			);

			// 여기서 맞은 대상이 누구인지 식별해서 데미지
			AActor* HitActor = HitResult.GetActor();
			if (HitActor)
			{
				// 예: GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Hit: %s"), *HitActor->GetName()));
			}
		}
	}
}

bool AC_GunBase::OnStartFire(AC_BasicPlayer* _WeaponUser)
{
	// TODO : LMB 첫 눌렸을 시, 동작 처리
	return false;
}

bool AC_GunBase::OnFireOnGoing(AC_BasicPlayer* _WeaponUser)
{
	// TODO : LMB 눌리고 있을 때의 동작 처리 (ex, 연발 사격 처리 등)
	return false;
}

bool AC_GunBase::OnFireEnd(AC_BasicPlayer* _WeaponUser)
{
	// TODO : LMB 떼었을 때 시점의 동작 처리(딱히 필요없으면 그냥 FireEnd 함수 Gun에서 지우시면 됩니다(동현))
	return false;
}

bool AC_GunBase::Reload(AC_BasicPlayer* _WeaponUser)
{
	// TODO : Reload 처리
	return false;
}
