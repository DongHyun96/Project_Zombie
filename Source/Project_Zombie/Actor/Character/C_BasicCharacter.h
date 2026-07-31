// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "C_BasicCharacter.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_BasicCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	AC_BasicCharacter();

protected:
	
	virtual void BeginPlay() override;

public:	
	
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage
	(
		float				_DamageAmount,
		FDamageEvent const& _DamageEvent,
		AController*		_EventInstigator,
		AActor*				_DamageCauser
	) override;

private:
	
	/// <summary>
	/// 서버 쪽으로 Damage를 입은 상황 알림 및 동기화 요청 (서버 쪽도 이 함수 호출시킬 것) 
	/// </summary>
	/// <param name="_DamageAmount"> : Damage량 </param>
	/// <param name="_DamageEvent"></param>
	/// <param name="_EventInstigatorActor"> : 기존의 TakeDamage와 맞추기 위함 -> Controller가 있을법한 Actor를 인자로 넘길 것</param>
	/// <param name="_DamageCauser"> : Damage 유발 Actor (무기 O / 무기를 사용하는 Character X) </param>
	UFUNCTION(Server, Reliable)
	void Server_TakeDamage
	(
		float				_DamageAmount,
		FDamageEvent const& _DamageEvent,
		AActor*				_EventInstigatorActor,
		AActor*				_DamageCauser
	);
	
public:
	
	class UC_StatComponentBase* GetStatComponent() const { return m_StatComponent; }
	
protected:

	// Player 및 Enemy 생성자에서 자기자신에게 맞는 StatComponent 생성 처리 중
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "StatComponent"))
	UC_StatComponentBase* m_StatComponent{};
};
