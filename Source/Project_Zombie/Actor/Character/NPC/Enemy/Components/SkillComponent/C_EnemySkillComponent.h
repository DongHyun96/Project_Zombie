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
	ESkillSlot								SlotType{};

	// 프라이머리 데이터에셋 비동기로딩 사용시 TSoftObjectPtr 사용
	// 에디터에서 설정할 데이터 에셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<class UC_EnemySkillData>	SkillData{}; 

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
	AC_BasicEnemy* m_OwnerEnemy{};
	
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

private:

	// 현재 Client단에서 Imitating 처리 중인 SkillSlot 
	// (서버 쪽 환경의 Enemy가 Loop가 걸린 Skill 모션을 직접 꺼버렸을 때, 클라이언트 환경에서도 직접 꺼야하는 처리가 들어가야한다)
	// 위의 상황에서 마지막으로 Imitate한 Skill 슬롯을 기억하여, 일치한다면 해당 동작을 꺼버릴 예정
	ESkillSlot m_CurImitatingSkillSlot{};
	
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
	/// 사망 시 현재 사용중인 스킬 상태를 강제로 종료
	/// BT Task 완료 Delegate는 호출하지 않음
	/// </summary>
	void CancelSkillForDead();

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

	/// <summary>
	/// 발사 스킬 사용시 호출
	/// </summary>
	void Fire();

	float GetSkillRange(ESkillSlot _Slot) const;

	/// <summary>
	/// 현재 사용중인 Skill의 Damage값 구하기 
	/// </summary>
	float GetCurSkillDamage() const;

private:
	
	/// <summary>
	/// Server환경 Enemy의 UseSkill이 성공한 경우, Client 환경에서 해당하는 Skill의 Montage 따라하기 처리 
	/// </summary>
	/// <param name="_ImitatingSkillSlot"> : 따라하려는 Skill의 Slot </param>
	/// <param name="_PlayedMontageSection"> : 서버 환경에서 재생된 Montage 내의 Section Index </param>
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ImitateUseSkill(ESkillSlot _ImitatingSkillSlot, int32 _PlayedMontageSection);

	/// <summary>
	/// Server환경 Enemy의 동작이 Manual하게 끊긴 상황(ex - Loop가 걸린 동작을 직접 StopMontage 처리)
	/// Client 환경에서도 직접 끊어주어야 비로소 Loop 동작이 끊기게 됨
	/// </summary>
	/// <param name="_TargetSkillSlot"> : 동작을 꺼버릴 TargetSlot 스킬 종류 </param>
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ImitateEndSkillManually(ESkillSlot _TargetSkillSlot);
	
public:
	UC_EnemySkillComponent();

};
