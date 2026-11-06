// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_InviteInteractable.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_InviteInteractable : public AActor
{
	GENERATED_BODY()

public:
	
	AC_InviteInteractable();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void InitExplanationWidgetTextField();

private:

	UFUNCTION()
	void BeginOverlap
	(
		UPrimitiveComponent* OverlappedComp,
		AActor*				 OtherActor,
		UPrimitiveComponent* OtherComp,
		int32				 OtherBodyIndex,
		bool				 bFromSweep,
		const FHitResult&	 SweepResult
	);
	
	UFUNCTION()
	void EndOverlap
	(
		UPrimitiveComponent* OverlappedComp,
		AActor*				 OtherActor,
		UPrimitiveComponent* OtherComp,
		int32				 OtherBodyIndex
	);

private:
	
	void ToggleOutlineEffect(bool _Enable);
	
protected:

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* m_MeshComponent{};
	
	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* m_OutlineMaterial{};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UC_FaceToLocalPlayerComponent* m_FaceToLocalPlayerComponent{};

	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* m_BoxComponent{};

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	class UWidgetComponent* m_ExplanationWidgetComponent{};

private:
	
	FTimerHandle m_InitTimerHandle{};
	
};
