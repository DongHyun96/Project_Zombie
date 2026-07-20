// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

// 비동기 로딩 관련 헤더
// #include "Actor/Character/NPC/Enemy/C_BasicEnemy.h"

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

public:
	// 슬롯 종류
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESkillSlot								SlotType;

	// 프라이머리 데이터에셋 비동기로딩 사용시 TSoftObjectPtr 사용
	// 에디터에서 설정할 데이터 에셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<class UC_EnemySkillData>	SkillData; 

	// Transient를 붙이면 에디터에 노출되지 않고 저장도 되지 않는다.
	// 런타임에 한번만 로드
	UPROPERTY(Transient)
	TObjectPtr<class UC_EnemySkillData> LoadedSkillData = nullptr;
	
	// 런타임에 한번만 생성
	UPROPERTY(Transient)
	TObjectPtr<class UC_EnemySkillBase> SkillInstance = nullptr;

};


DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillEnd, class AC_BasicEnemy*);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_EnemySkillComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	
	UPROPERTY()
	class AC_BasicCharacter* m_OwnerCharacter{}; // 이 SkillComponent의 주인 캐릭터 (일단 캐릭터 종류만 Skill 프레임워크를 사용할 수 있다고 판단)
	
protected:
	// 스킬 슬롯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (TitleProperty = "SlotType"))
	TArray<FSkillSlotInfo>						m_SkillSlots;

	// 스킬을 사용중인지 체크
	UPROPERTY()
	bool										bUsingSkill = false;

	// 현재 사용중인 스킬
	UPROPERTY(Transient)
	TSoftObjectPtr<class UC_EnemySkillData>			m_CurSkillData;

	// 쿨타임 기록 컨테이너
	// 스킬의 고유 ( 경로 + 카테고리명 ) 을 키값으로 사용한다
	// 스킬을 다시 사용할 수 있는 월드시간(ex: 월드시간 10초 + 쿨타임 5초 = 15초)
	TMap<FPrimaryAssetId, float>				m_mapSkillCoolTime;

public:
	// 스킬사용후 종료시 호출시켜줄 Delegate 들을 등록받을 수 있는 자료형
	FOnSkillEnd									m_SkillEndDelegate;

protected:
	virtual void BeginPlay() override;
	/// <summary>
	/// 게임 시작할 때 딱 한번만 실행되는 스킬 초기화 함수
	/// </summary>
	void InitializeSkills();

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	
	const TSoftObjectPtr<UC_EnemySkillData> GetCurSkillData() { return m_CurSkillData; }
	
public:
	
	/// <summary>
	/// 스킬이 사용중인지 체크하는 함수
	/// </summary>
	bool IsUsingSkill() const { return bUsingSkill; }

	/// <returns> : 스킬이 정상적으로 동작했다면 return true </returns>
	bool UseSkill(ESkillSlot _Slot);

	/// <summary>
	/// AnimNotify_EndSkill을 통해 EndSkill 처리가 되는 함수 
	/// </summary>
	void OnAN_EndSkill();

	/// <summary>
	/// <para> AnimNotify를 통한 EndSkill 처리를 하는 것이 아닌, 수동적으로 EndSkill 호출을 통해 Skill 끝맺음을 처리해아하는 경우 </para>
	/// <para> ex) -> 계속해서 Loop로 힐 주는 Skill 같은 경우, 해당 함수를 통해 EndSkill 처리를 해주어야 한다 </para>
	/// </summary>
	void EndSkillManually();

	/// <summary>
	/// 스킬을 사용할 수 있는지 검사하는 함수
	/// </summary>
	/// <param name="_Slot"></param>
	/// <returns> : 쿨타임중이면 false 반환
	bool CanUseSkill(ESkillSlot _Slot) const;

	/// <summary>
	/// 스킬 사용 후 쿨타임을 시작하는 함수
	/// </summary>
	void StartCooldown(ESkillSlot _Slot);

	/// <summary>
	/// 스킬의 남은 쿨타임을 초 단위로 반환하는 함수
	/// 디버깅 로그, 보스스킬UI, 쿨타임 표시, 쿨타임에 따라 다른 행동을 선택할 때 사용
	/// </summary>
	float GetRemainingCooldown(ESkillSlot _Slot) const;

	void Fire();

	float GetSkillRange(ESkillSlot _Slot) const;

public:
	UC_EnemySkillComponent();

};
