#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GlobalEnum.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GlobalData.generated.h" // UHT	

// 데이터 테이블로 관리할 아이템 정보
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    // ── [공통 정보] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Info")
    FText ItemName;

    // ── [공통 정보] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Info")
    FText ItemDescription;

    // ── [공통 정보] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Info")
    EItemType ItemType;

    // ── [공통 정보] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Info")
    int32 Count = 1;

    // ── [공통 정보] ── 인벤토리에서 겹쳐서 보관 할 수 있는 아이템인지.(true면 겹치기 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    bool bIsStack;

    // ── [비주얼 리소스 - 강참조 포인터] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual")
    TSoftObjectPtr<UTexture2D> IconTexture = nullptr;

    // ── [비주얼 리소스 - 강참조 포인터] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual")
    TSoftObjectPtr<UStaticMesh> DropMesh = nullptr;

    // ── [무기 전용 스펙 - 타 타입일 경우 무시] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Weapon Spec")
    float BaseDamage = 0.0f;

    // ── [무기 전용 스펙 - 타 타입일 경우 무시] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Weapon Spec")
    int32 BaseCurAmmo = 0;

    // ── [소모품 전용 스펙 - 타 타입일 경우 무시] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Consumable Spec")
    float HealAmount_HP = 0.0f;
};

// 인벤에 들어가 있는 아이템 정보
USTRUCT(BlueprintType)
struct FInventoryEntry : public FFastArraySerializerItem
{
    GENERATED_BODY()

public:
    // ── [네트워크 및 UI용 인덱스] ──
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    int32 SlotIndex = -1; //이 슬롯이 몇 번째 칸인지 기억하게 합니다.
    
    // ── [식별 정보] ──
    // 데이터 테이블에서 해당 아이템을 찾을 고유 키 (예: "Weapon_M4", "Consumable_Potion")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FName ItemRowName = NAME_None;

    // ── [실시간 공통 데이터] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Count = 0;

    // ── [실시간 공통 데이터] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    bool bIsStack;

    // ── [실시간 공통 데이터] ──
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    //bool bPicked = false;
    
    // ── [실시간 인스턴스 변수 - 무기/장비용] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory | Weapon")
    int32 UpgradeLevel = 0;

    // ── [실시간 인스턴스 변수 - 무기/장비용] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory | Weapon")
    int32 CurAmmo = 0;

    
    // ── [실시간 인스턴스 변수 - 무기/장비용] ──
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory | Weapon")
    //float Durability = 100.0f;
public:
    // 빈 슬롯인지 확인하는 함수.
    bool IsEmpty() const {return ItemRowName.IsNone() || Count == 0;}
    
    // 비우는 함수
    void Clear()
    {
        ItemRowName = NAME_None;
        Count = 0;
        bIsStack = false;
        UpgradeLevel = 0;
        CurAmmo = 0;
    }
};

USTRUCT(BlueprintType)
struct FCusorItem
{
    GENERATED_BODY()
    
    UPROPERTY()
    FInventoryEntry Item{};      // 현재 들고 있는 아이템
    
    // 어디서 가져왔는가?
    int32 SourceContainerID = -1;   // 상자의 ID 혹은 플레이어 인벤토리의 ID
    int32 SourceSlotIndex   = -1;     // 몇 번째 슬롯이었나?
    
    bool bIsValid = false;             // 현재 커서에 아이템이 들려있는가?
    
    void Clear()
    {
        Item.Clear();
        SourceContainerID = -1;
        SourceSlotIndex   = -1;
        bIsValid = false;
    }
};



// ******************************
// 무기 데이터테이블 구조체 선언부
// ******************************

// 데이터 테이블로 관리할 무기 공통 데이터
USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
    GENERATED_BODY()

    // ── [무기 종류 (Type)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
    EWeaponType WeaponType;

    // ── [무기 공통 스탯 (Stats)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float BaseDamage;     // 무기 기본 데미지

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRate;     // 총기 발사간격, 근접 공속, 투척 딜레이 공통 사용 (시간 간격 : 0.1초 = 초당 10발)

    // ── [무기 공통 매쉬 (Mesh)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<USkeletalMesh> WeaponSkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UStaticMesh> WeaponStaticMesh;


};

// 데이터 테이블로 관리할 총기 데이터
USTRUCT(BlueprintType)
struct FGunData : public FWeaponData
{
    GENERATED_BODY()

    // ── [총시 관련 스탯 (Stats)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxAmmo;     // 총기 최대 탄창 수

    // ── [총기 부가 설정] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ShellEjectImpulse = 150.0f; // 탄피 배출에 가하는 힘.

    // ── [총기 관련 매쉬 (Mesh)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UStaticMesh> ShellMesh;   // 총기 탄피 매쉬

    // ── [총기 애니메이션 (Animations)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TSoftObjectPtr<UAnimSequence> FireAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TSoftObjectPtr<UAnimSequence> ReloadAnimation;
};

USTRUCT(BlueprintType)
struct FMeleeData : public FWeaponData
{
    GENERATED_BODY()

    // ── [플레이어 근접무기 애니메이션 (Animations)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TSoftObjectPtr<UAnimMontage> PlayerAttackAnimation;
};

// ******************************
// 무기 데이터 테이블 구조체 선언부
// ******************************
