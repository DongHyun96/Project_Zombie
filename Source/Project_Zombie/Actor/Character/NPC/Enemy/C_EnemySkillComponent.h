// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

// 비동기 로딩 관련 헤더
#include "Engine/StreamableManager.h"

#include "C_EnemySkillComponent.generated.h"

UENUM(BlueprintType)
enum class ESkillSlot : uint8
{
	Skill_1		UMETA(DisplayName = "Skill 1"),
	Skill_2		UMETA(DisplayName = "Skill 2"),
	Skill_3		UMETA(DisplayName = "Skill 3"),
	END			UMETA(Hidden),
};

// 노출될 때 슬롯 타입과 스킬 데이터가 쉽게 알아볼 수 있도록 구조체로 묶어서 노출
USTRUCT(BlueprintType)
struct FSkillSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESkillSlot								SlotType;

	// 프라이머리 데이터에셋 비동기로딩 사용시 TSoftObjectPtr 사용
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<class UC_EnemySkillData>	SkillData; 
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_EnemySkillComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// 스킬 슬롯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (TitleProperty = "SlotType"))
	TArray<FSkillSlotInfo>			m_SkillSlots;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UC_EnemySkillComponent();

};
