// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "C_PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API AC_PlayerState : public APlayerState
{
	GENERATED_BODY()

	friend class AC_GameMode_GameLv;
	
public:
	
	AC_PlayerState();

private:
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	
	bool IsHost() const { return m_bIsHost; }
	
protected:
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool m_bIsHost{};
	
	
};

