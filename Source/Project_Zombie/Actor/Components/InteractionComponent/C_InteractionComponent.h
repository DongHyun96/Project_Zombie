#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_InteractionComponent.generated.h"



class AC_BasicPlayer;
class UC_InteractionStrategyBase;
class UPrimitiveComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UC_InteractionComponent();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/// <summary>
	/// 이 Actor 가 사용할 Collision과 Strategy 를 설정
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetupInteraction(UPrimitiveComponent* _InteractionCollision, TSubclassOf<UC_InteractionStrategyBase> _StrategyClass);
	
	/// <summary>
	/// 로컬 플레이어의 Collision에 Overlap 이벤트 연결
	/// </summary>
	void EnableInteractionDetection();

	/// =====================================
	/// 			전략 Strategy 정보
	/// =====================================
	/// <summary>
	/// 상호작용이 가능한가?
	/// </summary>
	bool CanBeInteractedBy(AC_BasicPlayer* _Interactor) const;

public:

	/// <summary>
	/// 상호작용 시도 (상호작용 시도한 플레이어의 InteractionComponent 에서 호출)
	/// </summary>
	void TryInteract();

	/// <summary>
	/// 상호작용 시도 (상호작용 Actor 의 InteractionComponent 에서 호출)
	/// </summary>
	bool ExecuteInteract(AC_BasicPlayer* _Interactor);

	/// <summary>
	///	상호작용 Actor
	/// </summary>
	AActor* GetFocusedTarget() const { return m_FocusedTarget.Get(); }
	

private: // Overlap 이벤트 처리

	UFUNCTION()
	void OnInteractionBeginOverlap(UPrimitiveComponent* _OverlappedComponent, AActor* _OtherActor, UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex, bool _bFromSweep, const FHitResult& _SweepResult);

	UFUNCTION()
	void OnInteractionEndOverlap(UPrimitiveComponent* _OverlappedComponent, AActor* _OtherActor, UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex);


private: // 타이머 설정

	/// <summary>
	/// FoucesdTarget 를 업데이트
	/// </summary>
	void UpdateFocusedTarget();

	void StartFocusUpdateTimer();

	void StopFocusUpdateTimer();

private:

	/// <summary>
	/// 설정된 클래스로 상호작용 전략 객체를 생성
	/// </summary>
	void CreateInteractionStrategy();

	/// <summary>
	/// 가장 적합한 상호작용 후보를 찾음
	/// </summary>
	AActor* FindBestInteractionTarget() const;

	/// <summary>
	/// 인터페이스에서 InteractionComponent 를 무조건 가져오도록 처리
	/// 인터페이스를 통해 상호작용 컴포넌트를 가져올 예정
	/// </summary>
	UC_InteractionComponent* GetTargetInteractionComponent(AActor* _TargetActor) const;

	/// <summary>
	///	플레이어와 대상 사이에 장애물이 없는지 검사
	/// </summary>
	bool HasClearLineOfSight(AActor* _TargetActor) const;

private:

	/// =====================================
	/// 			상호작용 전략
	/// =====================================
	// 상호작용 전략 클래스
	UPROPERTY(EditAnywhere, Category = "Interaction|Strategy")
	TSubclassOf<UC_InteractionStrategyBase> m_InteractionStrategyClass;

	// 실제 생성된 상호작용 전략 UObject
	UPROPERTY()
	TObjectPtr<UC_InteractionStrategyBase> m_InteractionStrategyObject;


	/// =====================================
	/// 			상호작용 Actor
	/// =====================================
	// TWeakObjectPtr 를 사용하여 Actor 가 사라졌을 때 안전하게 처리
	//	현재 InteractionSphere  와 겹쳐있는 상호작용 Actor 목록
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> m_InteractionCandidates;

	// 현재 가장 적합한 상호작용 Actor
	UPROPERTY(transient)
	TWeakObjectPtr<AActor> m_FocusedTarget;

	// InteractionComponent 를 소유한 플레이어
	// 아이템의 InteractionComponent 라면 nullptr
	UPROPERTY(Transient)
	TObjectPtr<AC_BasicPlayer> m_OwnerPlayer;

	/// <summary>
	/// Actor 가 상호작용을 위해 설정한 Collision Component
	/// </summary>
	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> m_InteractionCollision;

private:

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float m_FocusUpdateInterval;

private:

	/// 상호작용 완료 Timer
	FTimerHandle m_FocusUpdateTimerHandle;
};
