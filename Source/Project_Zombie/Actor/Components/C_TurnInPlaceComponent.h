// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_TurnInPlaceComponent.generated.h"


enum class EHandState : uint8;

USTRUCT(BlueprintType)
struct FTurnInPlaceMontages
{
	
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	UAnimMontage* TurnRightMontage{};
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	UAnimMontage* TurnLeftMontage{};
};

/// <summary>
/// Player 캐릭터 Speed 0일 때, 일정 각도 이상 마우스를 돌렸을 경우, Turn in place 모션으로 돌아가게끔 처리하는 Actor Component
/// TODO : 총기 Aim Down(견착 모드) 또는 Aim Down Sight(사이트 모드) 일 때의 예외적인 처리가 필요할 수 있음
/// </summary>
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_ZOMBIE_API UC_TurnInPlaceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UC_TurnInPlaceComponent();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	
	/// <summary>
	/// Turn in place가 끝났을 시 anim notify에 의해 호출, 멈춰 있을 때의 Rotation 세팅 값들로 돌아가기
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void SetStrafeRotationToIdleStop();

	/// <summary>
	/// 움직임이 시작되는 등의 행동이 이루어질 때, TurnInPlace 모션이 이미 재생중이었다면 해당 재생을 끊어주는 처리를 해야한다
	/// </summary>
	void CancelTurnInPlaceMotionIfNecessary();
	
private:
	
	/// <summary>
	/// <para> 캐릭터가 멈춰있을 때, 캐릭터가 오브젝트가 바라보는 방향과 컨트롤러(카메라)가 </para>
	/// <para> 바라보는 방향의 각이 90도 이상이면, Turn In place로 캐릭터 조정 </para>
	/// </summary>
	void HandleUpdateTurnInPlace(float DeltaTime);
	
private:
	
	class AC_BasicPlayer* m_OwnerPlayer{};

protected:
	
	// HandState에 따른 TurnInPlace 몽타주들 (현재는 Stand Pose 상태에서의 TurnInPlace 몽타주만 처리함) 
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TMap<EHandState, FTurnInPlaceMontages> m_TurnInPlaceMontages{};
	
};
