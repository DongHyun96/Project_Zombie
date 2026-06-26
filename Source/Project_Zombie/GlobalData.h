#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GlobalEnum.h"
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
struct FInventoryEntry
{
    GENERATED_BODY()

public:
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

    // ── [실시간 인스턴스 변수 - 무기/장비용] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory | Weapon")
    int32 UpgradeLevel = 0;

    // ── [실시간 인스턴스 변수 - 무기/장비용] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory | Weapon")
    int32 CurAmmo = 0;

    // ── [실시간 인스턴스 변수 - 무기/장비용] ──
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory | Weapon")
    //float Durability = 100.0f;
};

// ******************************
// 무기 데이터테이블 구조체 선언부
// ******************************

// ── [스탯 (Stats)] ──
USTRUCT(BlueprintType)
struct FGunStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FireRate = 0.1f; // 발사 간격 (초 단위)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxAmmo = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float					m_ShellEjectImpulse = 150.0f;
};

// ── [총기 매쉬 (Mesh)] ──
USTRUCT(BlueprintType)
struct FGunMesh
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh>   m_WeaponMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> m_ShellMesh;

};

// ── [애니메이션 (Animations)] ──
USTRUCT(BlueprintType)
struct FGunAnims
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UAnimSequence> m_FireAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UAnimSequence> m_ReloadAnimation;
};

// 데이터 테이블로 관리할 총기 정보
USTRUCT(BlueprintType)
struct FGunData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FGunStats Gun_Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    FGunMesh Gun_Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    FGunAnims Gun_Animations;

};

// ******************************
// 무기 데이터테이블 구조체 선언부
// ******************************
