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

	// Sphere Collision으로 충돌시 아이템 습득
	UFUNCTION(BlueprintCallable)
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);

	// 로드가 완료되면 호출될 콜백 함수 (반드시 ufunction이어야 합니다)
	UFUNCTION(BlueprintCallable)
	void OnMeshLoadCompleted(TSoftObjectPtr<UStaticMesh> LoadedSoftMesh);
public:	
	virtual void Tick(float DeltaTime) override;

	// 외부(매니저)에서 호출할 비동기 로드 시작 함수
	void SetPickupMeshAsync(TSoftObjectPtr<UStaticMesh> InSoftMesh);
public:
	// 아이템 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInventoryEntry ItemData;

	// 플레이어가 해당 아이템을 습득중인지 판별해줄 bool변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPickup = false;

protected:
	// 실제 땅과 충돌하고 날아다닐 물리 구체 (새로운 루트)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* PhysicsSphere;
	
	// 충돌을 감지할 구체 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* PickupSphere;

	// 바닥에 보일 아이템 메시
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	// 비동기 로드를 관리할 핸들러 포인터 유지
	TSharedPtr<struct FStreamableHandle> AssetLoadHandle;
};
