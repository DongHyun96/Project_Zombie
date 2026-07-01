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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillAnim")
	UAnimMontage* Montage; // 스킬 모션(동작)


public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("SkillData", GetFName());
	}
	
};
