// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/C_BasicCharacter.h"
#include "GenericTeamAgentInterface.h"

#include "C_BasicNPC.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_BasicNPC : public AC_BasicCharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info", meta = (DisplayName = "TeamID"))
	FGenericTeamId					m_TeamId;


public:
	AC_BasicNPC();
	
protected:
	virtual void BeginPlay() override;

	// TeamAgentInterface 
public:
	virtual void SetGenericTeamId(const FGenericTeamId& _NewId) override { m_TeamId = _NewId; }
	virtual FGenericTeamId GetGenericTeamId() const override { return m_TeamId; }


	/// <summary>
	/// 우호관계 적대관계 설정
	/// </summary>
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& _Other) const override;
	
};
