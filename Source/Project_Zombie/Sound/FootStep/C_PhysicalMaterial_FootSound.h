// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "C_PhysicalMaterial_FootSound.generated.h"

class USoundBase;

UCLASS()
class PROJECT_ZOMBIE_API UC_PhysicalMaterial_FootSound : public UPhysicalMaterial
{
	GENERATED_BODY()
	
public:
	UC_PhysicalMaterial_FootSound();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Walk")
	TObjectPtr<USoundBase> m_LeftWalkSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Walk")
	TObjectPtr<USoundBase> m_RightWalkSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Sprint")
	TObjectPtr<USoundBase> m_LeftSprintSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Sprint")
	TObjectPtr<USoundBase> m_RightSprintSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Landing")
	TObjectPtr<USoundBase> m_LandingSound;

public:
	// 걷는 소리 가져오기
	USoundBase* GetWalkSound(bool _IsLeftFoot) const { return  _IsLeftFoot ? m_LeftWalkSound : m_RightWalkSound; }

	// 달리는 소리 가져오기
	USoundBase* GetSprintSound(bool _IsLeftFoot) const { return  _IsLeftFoot ? m_LeftSprintSound : m_RightSprintSound; }

	// 착지 소리 가져오기
	USoundBase* GetLandingSound() const { return m_LandingSound; }
};
