// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Components/InteractionComponent/Interface/I_Interactable.h"
#include "GameFramework/Actor.h"
#include "GlobalEnum.h"
#include "C_InteractableBase.generated.h"


class UC_InvenComponent;
class USphereComponent;
class UC_InteractionComponent;
class UC_ItemUpgradeComponent;
class AC_BasicPlayer;

UCLASS()
class PROJECT_ZOMBIE_API AC_InteractableBase : public AActor, public II_Interactable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AC_InteractableBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual UC_InteractionComponent* GetInteractionComponent() const override { return InteractionComp; } 

	//UFUNCTION(Server, Reliable)

protected:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* m_MeshComp{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UC_InteractionComponent* InteractionComp{}; 
	
	// TODO : Sphere가 아니라 Cube로?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* m_SphereComp{};
};