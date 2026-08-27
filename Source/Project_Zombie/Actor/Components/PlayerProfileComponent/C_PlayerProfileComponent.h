// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_PlayerProfileComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_ZOMBIE_API UC_PlayerProfileComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UC_PlayerProfileComponent();

protected:
	
	virtual void BeginPlay() override;

public:
	
	const FColor& GetPlayerSelectedColor() const { return m_PlayerSelectedColor; }

public:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	
	UFUNCTION()
	void OnRep_PlayerSelectedColor();
	
private:
	
	UPROPERTY()
	class AC_BasicPlayer* m_OwnerPlayer{};
	
protected:

	UPROPERTY(ReplicatedUsing = OnRep_PlayerSelectedColor, VisibleAnywhere, BlueprintReadOnly, Category = "PlayerProfile", meta = (DisplayName = "PlayerSelectedColor"))
	FColor m_PlayerSelectedColor{};
	
private:

	FTimerHandle m_OtherPlayerRegTimerHandle{};
	
};
