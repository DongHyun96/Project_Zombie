// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "C_EnemySkillData.generated.h"

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	MELEE,
	PROJECTILE,
	BUFF,
};

UCLASS()
class PROJECT_ZOMBIE_API UC_EnemySkillData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	ESkillType SkillType; // 스킬 타입

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	TSubclassOf<class UC_EnemySkillBase> SkillClass; // 스킬 클래스

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	FName SkillName; // 스킬 이름

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	FText DisplayName; // 스킬 이름(현지화 버전)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	FText Description; // 스킬 설명(현지화 버전)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	float Damage; // 스킬 데미지

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	float CoolTime; // 스킬 재사용에 걸리는 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	float Range; // 스킬 유효 사거리

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	bool CanMove; // 이동중에 사용 가능여부

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	bool CanCombo; // 연계동작이 있는 스킬인가

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	bool CanJump; // 공중에서 사용 가능여부

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	float MoveSpeedScale; // 스킬 사용시 이동속도 배율

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	float RotateSpeed; // 스킬 사용시 캐릭터 방향회전 속도
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillInfo")
	bool bRotateToTargetOnActivation = true; // 스킬 실행 시작 시, 바로 Target을 향해 돌지 결정	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillAnim")
	UAnimMontage* Montage; // 스킬 모션(동작)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillSound")
	USoundBase* FireSound; // 발사 사운드 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillSound")
	USoundBase* HitSound; // 투사체 충돌 사운드

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "lProjectile")
	TSubclassOf<class AC_EnemyProjectile> ProjectileClass; // 생성시킬 투사체

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectileSpeed; // 투사체 속도

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectileLifetime; // 투사체 수명

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillKnockback")
	float KnockbackPower = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillKnockback")
	float KnockbackUpPower = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillKnockback")
	float FriendlyKnockbackPower = 800.f;

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("SkillData", GetFName());
	}
	
}; 
