// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_PointTowerElectroEffect.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_PointTowerElectroEffect : public AActor
{
	GENERATED_BODY()

public:
	
	AC_PointTowerElectroEffect();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

public:

	void SetTargetPlayer(class AC_BasicPlayer* _TargetPlayer);

private:
	
	UFUNCTION()
	void OnRep_TargetPlayer();
	
private:
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
private:

	UPROPERTY(Replicated)
	class AC_PointTower* m_OwnerPointTower{};	
	
	UPROPERTY(ReplicatedUsing = OnRep_TargetPlayer)
	AC_BasicPlayer* m_TargetPlayer{};

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class USplineComponent* m_Spline{};	
		
};


