// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/NPC/C_BasicNPC.h"
#include "C_BasicEnemy.generated.h"

class UC_ItemManager;

/// <summary>
/// 서버에서 결정한 죽음 상태와 몽타주 정부를
/// 클라이언트에도 전달하기 위한 구조체
/// m_bDead와 몽타주 인덱스의 복제 순서를 
/// 보장하기 위해 구조체로 묶어서 RepNotify 처리
/// </summary>
USTRUCT()
struct FEnemyDeadRepData
{
	GENERATED_BODY()

public:
	// 현재 죽은 상태인지 여부
	UPROPERTY()
	bool bDead = false;

	// 서버에서 선택한 죽음 몽타주 배열 인덱스
	UPROPERTY()
	int32 DeadMontageIndex = INDEX_NONE;
};

UCLASS()
class PROJECT_ZOMBIE_API AC_BasicEnemy : public AC_BasicNPC
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "SkillComponent"))
	class UC_EnemySkillComponent*			m_SkillCom{};
	
	// 이 몬스터가 사망 시 참조할 드랍 테이블 데이터 에셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
	TObjectPtr<class UC_DropTableDataAsset> m_DropTableDataAsset{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitAnim")
	TArray<TObjectPtr<UAnimMontage>> m_HitMontage;

protected: /* Dead 관련 */
	/// <summary>
	/// 서버에서 결정한 죽음 상태와 몽타주 인덱스를
	/// 클라이언트에서 받으면 OnRep_DeadData() 호출해서 처리
	/// </summary>
	UPROPERTY(ReplicatedUsing = OnRep_DeadData)
	FEnemyDeadRepData m_DeadRepData;

	// 죽음 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dead")
	TArray<TObjectPtr<UAnimMontage>> m_DeadMontages{};

	// 죽은 뒤 풀 반환까지의 대기시간(월드에 남아있는 시간)
	// ClampMin : 에디터에서 음수를 넣지 못하게 막는 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dead", meta = (ClampMin = "0.0"))
	float m_DeadRemainTime = 3.f;

	// 죽은 뒤 풀 반환까지 기다리는 타이머
	FTimerHandle m_DeadRemainTimer;

protected:
	
	UPROPERTY()
	class AC_ZombieController* m_ZombieController{};

private:
	
	// ItemManager Subsystem 캐싱 
	UPROPERTY()
	TObjectPtr<UC_ItemManager> m_ItemManager{};
	
protected:
	
	// 힐을 받았을 때 활성화시킬 NiagaraEffectComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Niagara")
	class UNiagaraComponent* m_HealedEffectNGComponent{};

private: /* 자기 자신을 힐러에게 힐등록할 수 있는 갯수 제한 관련 */
	
	static const int8 s_MaxHealRequestRegisterCount; // 최대 힐 Register 등록 가능 횟수 (2회(또는 2마리)로 제한)
	int8 m_HealRequestRegisterCount{}; // 힐 요청 Request 등록 count
	
public:
	
	AC_BasicEnemy();

protected:
	
	virtual void BeginPlay() override;
	
public:

	virtual float TakeDamage
	(
		float				_DamageAmount,
		FDamageEvent const& _DamageEvent,
		AController*		_EventInstigator,
		AActor*				_DamageCauser
	) override;

public:
	
	UC_EnemySkillComponent* GetSkillComponent() const { return m_SkillCom; }

	AC_ZombieController* GetZombieController() const { return m_ZombieController; }
	
private:
	
	void OnHPIncreased(AC_BasicCharacter* _HPIncreasedCharacter);

	/// <summary>
	/// 힐 받았을 때 Effect 동기화 시 호출처리
	/// </summary>
	/// <param name="_Activate"></param>
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ToggleHealedEffect(bool _Activate);

	void DropItemOnDead();

protected: // ---- 죽음 관련 ---- //
	
	/// <summary>
	/// 사망 절차만 처리
	/// 사망 시 호출받는 Delegate -> HealedEffect 활성화 중이었다면 해당 Effect 끄기 (및 기타 처리 여기서 할 것)
	/// </summary>
	/// <param name="_DeadCharacter"> : 죽은 캐릭터 (자기자신) </param>
	virtual void OnDead(AC_BasicCharacter* _DeadCharacter);

	/// <summary>
	/// 죽음 연출이 끝난 후(OnDead 가 끝난 후) ZombieManager에 Pool 반환 요청
	/// </summary>
	UFUNCTION()
	virtual void FinishDead();

	/// <summary>
	/// AI와 이동 정지
	/// </summary>
	virtual void StopAllActionsForDead();

	/// <summary>
	/// 서버와 클라이언트에서 공통으로 적용할 죽음 처리
	/// 몽타주 정지, 죽음 몽타주 재생, 충돌 비활성화
	/// </summary>
	void ApplyDeadState(int32 _DeadMontageIndex);

	/// <summary>
	/// 전달받은 인덱스의 죽음 몽타주 재생
	/// 서버와 클라이언트 공통 시각 처리
	/// </summary>
	virtual void PlayDeadAnimation(int32 _DeadMontageIndex);

	/// <summary>
	/// 클라이언트가 서버에서 결정된 죽음 상태와
	/// 몽타주 인덱스를 받으면 호출되는 RepNotify 함수
	/// </summary>
	UFUNCTION()
	void OnRep_DeadData();

	// 사망 음성
	virtual void PlayDeadSound() {}

protected:
	/// <summary>
	/// 풀에서 다시 활성화될 때
	/// 죽음 상태와 좀비 공통 상태값을 초기화
	/// 서버에서만 호출
	/// </summary>
	virtual void ResetEnemyForPoolSpawn();

protected: /* 피격처리 */
	
	int32 SelectHitMontageIndex() const;

	void PlayHitAnimation(int32 _Idx);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHit(int32 _HitMontageIdx);

	// 피격 음성
	virtual void PlayHitSound() {}

	// 추격 음성
	virtual void PlayChaseSound() {}
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	
	void DecreaseHealRequestRegisterCount();

};
