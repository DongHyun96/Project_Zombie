// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalData.h"
#include "Components/ActorComponent.h"
#include "C_ItemLinkComponent.generated.h"

//class FInventoryEntry;
class UC_InvenComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_ItemLinkComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UC_ItemLinkComponent();

protected:
	// 연결된 인벤토리 컴포넌트 참조 (장착창 InvenComp 등)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Link")
	TObjectPtr<UC_InvenComponent> OwningInvenComp;

	// 해당 인벤토리에서의 슬롯 인덱스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Link")
	int32 SlotIndex = INDEX_NONE;

public:
	// 장착/스폰 시점에 데이터 원본과의 연결 초기화, 
	// TODO : invencomponent와 연결하는 것과 조금 돌아가지만 붙어 있는 액터의 OwnerPlayer를 변수로 가지고 있는게 주소값이 안전한가?
	// TODO : 근데 그렇게 안전한 주소값으로는 PlayerController가 더 나아 보이고.
	// TODO : 안전으로 따지면 그냥 PlayerID 같은걸로 해서 항상 해당 아이디의 플레이어를 찾아 인벤을 찾아가는게 제일 안전하지 않나?
	UFUNCTION(BlueprintCallable, Category = "Item Link")
	void InitializeLink(UC_InvenComponent* InInvenComp, int32 InSlotIndex);

	// 연결 해제 (무기 파괴/버리기 시)
	UFUNCTION(BlueprintCallable, Category = "Item Link")
	void ClearLink();

	// 연결 상태 유효성 확인
	UFUNCTION(BlueprintCallable, Category = "Item Link")
	bool IsLinkValid() const;

	// 원본 FInventoryEntry 데이터를 포인터로 직접 반환 (읽기/수정용)
	FInventoryEntry* GetItemEntryPtr() const;

	// 원본 데이터 복사본 반환 (안전한 읽기 전용), TODO : 뭐지 함수의 반환값은 구현부쪽에 헤더파일 추가해주면 상관없나?
	UFUNCTION(BlueprintCallable, Category = "Item Link")
	FInventoryEntry GetItemEntry() const;

	// Getter
	UC_InvenComponent* GetOwningInvenComp() const { return OwningInvenComp; }
	int32 GetSlotIndex() const { return SlotIndex; }
		
};
