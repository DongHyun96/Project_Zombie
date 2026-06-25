// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GlobalData.h"
#include "C_ItemPickUp.generated.h"

UCLASS()
class PROJECT_ZOMBIE_API AC_ItemPickUp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AC_ItemPickUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 아이템 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInventoryEntry ItemData;

	// 플레이어가 해당 아이템을 습득중인지 판별해줄 bool변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPickup = false;

protected:
	// 충돌을 감지할 구체 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* CollisionSphere;

	// 바닥에 보일 아이템 메시
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MeshComp;
};
