// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GlobalData.h"
#include "C_ItemPickUp.generated.h"

// 스폰되고 플레이어와 바로 오버랩되지 않고 DelayTime 이후에 오버랩 기능이 켜져서 오버랩(아이템 파밍)가능.
#define DELAYTIME 2.f

class AC_BasicPlayer;

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
	
	void SetMeshRef(TSoftObjectPtr<UStaticMesh> InMeshRef) { MeshRef = InMeshRef; }
	
	TSoftObjectPtr<UStaticMesh> GetMeshRef() { return MeshRef; }
	
	void EnablePickupOverlap();
	
	void ActivateItem(const FInventoryEntry& InEntry, const FVector& SpawnLocation);
	void DeactivateItem();

	// 일정 시간 후 자동 수거 타이머 설정
	void StartDespawnTimer(float InLifeTime = 30.0f);
	
	// 서버 함수
public:
	
	UFUNCTION(Server, Reliable)
	void Server_RequestPickup(AC_BasicPlayer* Player);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_MeshRef();
	
	UFUNCTION()
	void OnRep_ItemEntry();
	
private:
	
	/// <summary>
	/// 스폰 처리 후, 구르는 것 멈췄는지 체크
	/// 멈췄다면 SimulatePhysics 비활성화 및 CopZombie가 떨군 ItemPickUp의 경우, Outline 그리기 처리 등 조치
	/// </summary>
	void CheckPhysicsStopped();
	
public:
	
	void SetStolenPlayerPingSystemComponent(class UC_PingSystemComponent* _TargetPingComponent) { m_StolenPlayerPingSystemComponent = _TargetPingComponent; }

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ToggleOutline(bool _Visible);
	
public:
	// 아이템 정보
	UPROPERTY(ReplicatedUsing = OnRep_ItemEntry, EditAnywhere, BlueprintReadWrite)
	FInventoryEntry ItemEntry;

	// 플레이어가 해당 아이템을 습득중인지 판별해줄 bool변수
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
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

	// 클라이언트가 이 변수가 변경될 때마다 메시를 로드합니다. TODO : 이거 꼭 필요한가?
	UPROPERTY(ReplicatedUsing = OnRep_MeshRef, EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> MeshRef;
	
	// 비동기 로드를 관리할 핸들러 포인터 유지
	TSharedPtr<struct FStreamableHandle> AssetLoadHandle;
	
	// 생성되고 일정 시간 동안 PcikUp할 수 없도록 막음.
	FTimerHandle PickupDelayTimerHandle;
	
	// 자동 디스폰 타이머 핸들
	FTimerHandle DespawnTimerHandle;

	// 자동 수거 시간 (초 단위)
	UPROPERTY(EditAnywhere, Category = "Item | LifeTime")
	float DefaultLifeTime = 100.0f;

protected: /* CopZombie에 의한 Drop 처리 이후 관련 */
	
	FTimerHandle m_SimulatePhysicsStoppedCheckTimer{};

	// CopZombie에 의해 빼앗긴 무기의 경우, 이 PingSystemComponent 등록할 것
	// 만약 SimulatePhysics에 의한 움직임이 멈춘 시점에 해당 Component가 존재한다면
	// 빼앗긴 무기에 대한 회수 시인성 정보 표시처리를 해줄 것임
	UPROPERTY()
	class UC_PingSystemComponent* m_StolenPlayerPingSystemComponent{};
	
};
