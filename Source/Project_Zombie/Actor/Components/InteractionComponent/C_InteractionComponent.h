#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_InteractionComponent.generated.h"


class AC_BasicPlayer;
class UC_InteractionStrategyBase;
class UPrimitiveComponent;
class UMaterialInterface;

// TODO : CancelInteract 구현 보고 없애야 할 수 있음.
DECLARE_MULTICAST_DELEGATE(OnEndOverlap);
// TODO : CancelInteract 구현 보고 없애야 할 수 있음.
DECLARE_DELEGATE_OneParam(OnEndOverlapParm1, bool);

UENUM(BlueprintType)
enum class EInteractionNetType : uint8
{
	Local,
	Server
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UC_InteractionComponent();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 리플리케이트 할 변수 등록
	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	/// <summary>
	/// 동시 사용 가능한 객체인가?
	/// </summary>
	/// <param name="_bAllow"></param>
	void SetAllowMultipleInteractor(bool _bAllow) { m_AllowMultipleInteractor = _bAllow; }

	/// <summary>
	/// Timer핸들 사용 여부 설정
	/// </summary>
	/// <param name="_bUseTimer"></param>
	///void SetUseTimer(bool _bUseTimer) { bUseTimer = _bUseTimer; }

	/// <summary>
	/// 상호작용	네트워크 처리 방식 설정
	/// </summary>
	/// <param name="_NetType"></param>
	void SetInteractionNetType(EInteractionNetType _NetType) { m_InteractionNetType = _NetType; }

	/// <summary>
	/// 이 Actor 가 사용할 Collision과 Strategy 를 설정
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetupInteraction(UPrimitiveComponent* _InteractionCollision);
	
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

	/// <summary>
	/// 현재 다른 Actor 와 상호작용 중인지 확인
	/// </summary>
	bool HasCurrentInteraction() const { return m_CurrentInteractionTarget.IsValid(); }

	/// <summary>
	/// 현재 상호작용 중인 상대 Actor 반환
	/// </summary>
	AActor* GetCurrentInteractionTarget() const { return m_CurrentInteractionTarget.Get(); }


	EInteractionNetType GetInteractionNetType() const { return m_InteractionNetType; }

	bool IsAllowMultipleInteractor() const { return m_AllowMultipleInteractor; }

	float GetInteractionDuration() const;

	// 나중에 전략 Strategy 에서 상호작용 텍스트를 가져오도록 처리해도 될듯
	//const FText& GetInteractionText() const { return m_InteractionStrategyObject ? m_InteractionStrategyObject->GetInteractionText() : FText::GetEmpty(); }
	const FText& GetInteractionText() const { return m_InteractionText; }

	
	bool GetbUseTimer() {return bUseTimer;}
public:

	/// <summary>
	/// 상호작용 시도 (상호작용 시도한 플레이어의 InteractionComponent 에서 호출)
	/// </summary>
	void TryInteract();

	/// <summary>
	/// 상호작용 취소
	/// </summary>
	void CancleInteract();

private:
	/// <summary>
	/// 상호작용 시도 (상호작용 Actor 의 InteractionComponent 에서 호출)
	/// </summary>
	/// <param name="_Interactor">상호작용 하는 Player</param>
	bool ExecuteInteract(AC_BasicPlayer* _Interactor);

	/// <summary>
	/// 상호작용 취소 (상호작용 Actor 의 InteractionComponent 에서 호출)
	/// </summary>
	/// <param name="_Interactor">상호작용 하는 Player</param>
	bool ExecuteCancleInteract(AC_BasicPlayer* _Interactor);

	/// <summary>
	/// 상호작용 완료 (상호작용 시도한 플레이어의 InteractionComponent 에서 호출)
	/// </summary>
	void CompleteInteract();

	/// <summary>
	/// 상호작용 완료 (상호작용 Actor 의 InteractionComponent 에서 호출)
	/// </summary>
	/// <param name="_Interactor">상호작용 하는 Player</param>
	bool ExecuteCompleteInteract(AC_BasicPlayer* _Interactor);

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

	/// <summary>
	/// Interactable 인터페이스를 구현한 Actor 와 상호작용 Timer 업데이트
	/// </summary>
	void StartInteractionTimer(float _Duration);

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

	/// <summary>
	/// CurrentInteractionActor 를 초기화
	/// </summary>
	void ClearCurrentInteraction();

	/// <summary>
	/// 아웃라인 효과
	/// </summary>
	/// <param name="_Enable"></param>
	void SetOutlineEffect(bool _Enable);

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
	UPROPERTY(Transient)
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

	/// <summary>
	/// 내가 현재 상호작용 중인 Actor
	/// Player 의 InteractionComponent 에서 사용
	/// </summary>
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> m_CurrentInteractionTarget;

	/// <summary>
	/// 현재 나와 상호작용하고 있는 Player 목록
	/// 상호작용되는 Actor 의 InteractionComponent 에서 사용
	/// </summary>
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AC_BasicPlayer>> m_CurrentInteractors;


	// Server
private:
	UFUNCTION(Server, Reliable)
	void Server_TryInteract(AActor* _TargetActor);

	UFUNCTION(Server, Reliable)
	void Server_CancleInteract(AActor* _TargetActor);

	UFUNCTION(Client, Reliable)
	void Client_SetCurrentInteractionTarget(AActor* _TargetActor);

private:

	// 상호작용 시도 시 서버에 요청할지, 로컬에서 처리할지 결정
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	EInteractionNetType m_InteractionNetType;

	// 상호작용 시도 시 다른 플레이어와 동시에 상호작용 가능 여부
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	bool m_AllowMultipleInteractor;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float m_FocusUpdateInterval;

	// ======================================
	// 			아웃라인 효과
	// ======================================

	// 아웃라인을 적용할 Mesh 들을 미리 기억해둠
	// 처음 한번만 찾고 그 이후에는 캐싱된 Mesh 들을 사용
	UPROPERTY(EditAnywhere, Category = "Interaction|Outline")
	TArray<TObjectPtr<UMeshComponent>> m_OutlineMeshComponents;

	UPROPERTY(EditAnywhere, Category = "Interaction|Outline")
	UMaterialInterface* m_OutlineMaterial;

	// ======================================
	// 				UI 관련
	// ======================================

	UPROPERTY(EditAnywhere, Category = "Interaction|UI")
	FText m_InteractionText;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Interaction|UI")
	bool bUseTimer = true;
	
private:


	// 상호작용 걸리는 시간
	FTimerHandle m_InteractionTimerHandle;

	/// 상호작용 완료 Timer
	FTimerHandle m_FocusUpdateTimerHandle;
	
public:
	// TODO : CancelInteract 구현 보고 없애야 할 수 있음.
	OnEndOverlap m_OnEndOverlap;
	// TODO : CancelInteract 구현 보고 없애야 할 수 있음.
	OnEndOverlapParm1 m_OnEndOverlapParm1;
};
