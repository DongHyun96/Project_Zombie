// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../C_WeaponBase.h"
#include "C_GunBase.generated.h"

// 상황 : 이 모든 FireMode를 지원하지 않는 총기일 수 있음
// TArray<EFireMode> m_ApplicableFireMode -> 해당 배열에 총기 자신이 지원하는 FireMode를 넣고, EFireMode m_CurrentFireMode -> 이런식으로 둘 수 있을 듯
/// <summary>
/// 총기 조정간 FireMode Type
/// </summary>
UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Single,		// 단발
	Burst,		// 점사
	FullAuto,	// 연발
	End			UMETA(Hidden) 
};

UCLASS()
class PROJECT_ZOMBIE_API AC_GunBase : public AC_WeaponBase
{
	GENERATED_BODY()

protected:
	// 최대 총알 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data", meta = (ClampMin = "1"))
	int32					m_MaxAmmo = 30;        

	// 연사 속도 (발사 간의 시간 간격 : 0.1초 = 초당 10발)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data")
	float					m_FireRate = 0.1f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon|Mesh")
	USkeletalMeshComponent* m_WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	UAnimSequence*			m_FireAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	UAnimSequence*			m_ReloadAnimation;

	// 에디터에서 등록할 탄피 스태틱 메시 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	UStaticMesh*			m_ShellMesh;

	// 탄피가 배출구 소켓 기준으로 어느 방향으로 튈지 더해줄 오프셋 힘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	float					m_ShellEjectImpulse = 150.0f;

protected:
	//현재 남아있는 총알 수
	int32					m_CurrentAmmo;

	// 연사 타이머를 관리하기 위한 핸들
	FTimerHandle			m_FireTimerHandle;

	// 현재 사격 버튼을 누르고 있는 상태인지 확인
	bool m_bIsFiring =		false;

public:
	/// <summary>
	/// 마우스 왼쪽 버튼 클릭 (사격 시작)
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void PullTrigger();

	/// <summary>
	/// 마우스 왼쪽 버튼 클릭 뗌 (사격 중지)
	/// <summary>
	UFUNCTION(BlueprintCallable)
	void ReleaseTrigger();

	/// <summary>
	/// R 키 입력 (재장전 요청)
	/// <summary>
	UFUNCTION(BlueprintCallable)
	void Gun_Reload();

	/// <summary>
	/// 방아쇠를 당겼을 때 애니메이션 재생 함수
	/// <summary>
	UFUNCTION(BlueprintCallable)
	void CompleteReload();

	void PlayFireEffects();

public:
	virtual void StartAttack() override;
	virtual void StopAttack() override;
	virtual void Reload() override;
	
	virtual bool AttachToHand(USceneComponent* _ParentMesh) override;
    virtual bool AttachToHolster(USceneComponent* _ParentMesh) override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	AC_GunBase();
};
