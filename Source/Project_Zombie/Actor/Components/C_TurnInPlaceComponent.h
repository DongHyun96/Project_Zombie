// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_TurnInPlaceComponent.generated.h"


enum class EHandState : uint8;


/// <summary>
/// <para> HandState 별 TurnInPlace 모션 담을 struct </para>
/// <para> Default group full body / Addit group lower body slot 모두 포함 </para>
/// </summary>
USTRUCT(BlueprintType)
struct FTurnInPlaceMontages
{
	GENERATED_BODY()

	// Stand Default Group full body slot && Addit Group Lower body slot TurnInPlace 몽타주가 들어간다. 
	UPROPERTY(VisibleAnywhere)
	TArray<UAnimMontage*> TurnRightMontages{};
	
	UPROPERTY(VisibleAnywhere)
	TArray<UAnimMontage*> TurnLeftMontages{};
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
	
	/// <summary>
	/// 움직임이 시작되는 등의 행동이 이루어질 때, TurnInPlace 모션이 이미 재생중이었다면 해당 재생을 끊어주는 처리를 해야한다
	/// </summary>
	void CancelTurnInPlaceMotionIfNecessary();

	/// <summary>
	/// Turn in place 모션 재생 처리 시작 (Local)
	/// </summary>
	/// <param name="_IsRight"> : TurnInPlace 방향 </param>
	/// <param name="_RequestToServer"> : 재생 성공 시 서버에 해당 모션 동기화 요청을 넣을 건지 </param>
	/// <returns> : 처리할 수 없다면 return false </returns>
	bool StartTurnInPlaceMotion(bool _IsRight, bool _RequestToServer = true);
	
private:
	
	/// <summary>
	/// Server 쪽 TurnInPlace 모션 request (로컬 플레이어는 자기자신의 캐릭터 미리 turnInPlace 동작 처리)
	/// </summary>
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestTurnInPlaceMotion(bool _IsRight);

	UFUNCTION(Server, Reliable)
	void Multicast_StartTurnInPlaceMotion(bool _IsRight);

private:
	
	/// <summary>
	/// 멤버변수 초기화용 
	/// </summary>
	void InitTurnInPlaceMontages();
	
private:
	
	class AC_BasicPlayer* m_OwnerPlayer{};

protected:
	
	// HandState에 따른 Stand TurnInPlace 몽타주들 
	UPROPERTY(VisibleAnywhere)
	TMap<EHandState, FTurnInPlaceMontages> m_StandTurnInPlaceMontages{}; 
	
	UPROPERTY(VisibleAnywhere)
	TMap<EHandState, FTurnInPlaceMontages> m_CrouchTurnInPlaceMontages{};	
	
};
