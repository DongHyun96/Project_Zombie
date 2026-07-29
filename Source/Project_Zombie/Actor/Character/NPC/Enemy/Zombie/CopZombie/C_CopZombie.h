// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/NPC/Enemy/Zombie/C_Zombie.h"
#include "C_CopZombie.generated.h"

UENUM(BlueprintType)
enum class ECopZombieState : uint8
{
	Idle,
	WeaponEarned,
	End UMETA(Hidden)
};

UCLASS()
class PROJECT_ZOMBIE_API AC_CopZombie : public AC_Zombie
{
	GENERATED_BODY()

public:
	
	AC_CopZombie();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

public:
	
	ECopZombieState GetCopZombieState() const { return m_CopZombieState; }
	void SetCopZombieState(ECopZombieState _State) { m_CopZombieState = _State; }

	const TSet<class AC_BasicPlayer*>& GetGrabRangeEnteredPlayers() const { return m_GrabRangeEnteredPlayers; }

	class AC_GunBase* GetEquippedGun() const { return m_EquippedGun; }
	
public:

	UFUNCTION(BlueprintCallable)
	void OnANSGrabStart();
	
	UFUNCTION(BlueprintCallable)
	void OnANSGrabEnd();	
	
private:
	
	UFUNCTION()
	void OnGrabRangeColliderBeginOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor*				 OtherActor,
		UPrimitiveComponent* OtherComp,
		int32				 OtherBodyIndex,
		bool				 bFromSweep,
		const FHitResult&	 SweepResult
	);

	UFUNCTION()
	void OnGrabRangeColliderEndOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor*				 OtherActor,
		UPrimitiveComponent* OtherComp,
		int32				 OtherBodyIndex
	);

public:
	
	/// <summary>
	/// 강탈한 무기 자기자신에게 장착 
	/// </summary>
	/// <returns> : 장착 실패 시 return false </returns>
	bool EquipWeapon(AC_GunBase* _StolenGun);

	/// <summary>
	/// 현재 들고있는 무기 내려놓기 
	/// </summary>
	void DropWeapon();

private:

	UFUNCTION()
	void OnNormalAttackColliderBeginOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor*				 OtherActor,
		UPrimitiveComponent* OtherComp,
		int32				 OtherBodyIndex,
		bool				 bFromSweep,
		const FHitResult&	 SweepResult
	);

private:
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	ECopZombieState m_CopZombieState{};

	// 주무기를 뺏을 수 있는 영역 Collider
	// 이 영역에 실질적인 Player가 들어와 있어야 해당 Player의 MainWeapon을 뺏을 수 있다고 판별
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UBoxComponent* m_GrabRangeCollider{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UBoxComponent* m_NormalAttackCollider{};
	
protected:

	// Grab Range에 들어온 Player들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSet<AC_BasicPlayer*> m_GrabRangeEnteredPlayers{};
	
protected: // 빼앗아서 장착한 무기
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	AC_GunBase* m_EquippedGun{};
	
private:
	
	UPROPERTY()
	TSet<AC_BasicPlayer*> m_NormalAttackColliderEntered{};

};
