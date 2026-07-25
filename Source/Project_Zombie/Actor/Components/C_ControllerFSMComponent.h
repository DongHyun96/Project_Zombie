// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_ControllerFSMComponent.generated.h"


/// <summary>
/// 상황 별, Player Controller Rotation 관련 값이 상이함
/// </summary>
UENUM(BlueprintType)
enum class EPlayerControllerRotState : uint8
{
	IdleStopState,		// 가장 기본 가만히 있는 상태
	TurnInPlaceState,	// DeltaYaw의 Abs값이 90도를 넘어간 상태
	MovingState,		// 움직이거나 점프하거나 떨어지는 상태
	FreeLookState		// 시점 고정 상태
};

/// <summary>
/// Player 각 상황 별 Rotation 상태 전이 처리 담당 component
/// </summary>
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_ZOMBIE_API UC_ControllerFSMComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UC_ControllerFSMComponent();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	
	/// <summary>
	/// TurnInPlace 모션 이후, IdleStopState Transition 전환 처리 여기서
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void OnTurnInPlaceFin();
	
private:
	
	/// <summary>
	/// State machine 내의 상태 전이 처리 
	/// </summary>
	void HandleFSMTransition();

private:
	
	void SetControllerRotState(EPlayerControllerRotState _NewRotState);

	UFUNCTION(Server, Reliable)
	void Server_SetControllerRotState(EPlayerControllerRotState _NewRotState);
	
	/// <summary>
	/// 현재 State에 따라 각 캐릭터 Rotation 값 set 
	/// </summary>
	void SetEachRotValueByCurState();

	UFUNCTION()
	void OnRep_PlayerControllerRotState();

public:
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	
	class AC_BasicPlayer*				m_OwnerPlayer{};
	class UCharacterMovementComponent*	m_PlayerMovement{};

protected:
	
	UPROPERTY(ReplicatedUsing = "OnRep_PlayerControllerRotState", VisibleAnywhere, BlueprintReadOnly)
	EPlayerControllerRotState m_PlayerControllerRotState{};

};
