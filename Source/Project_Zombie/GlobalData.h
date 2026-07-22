#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GlobalEnum.h"
#include "Net/Serialization/FastArraySerializer.h"
//#include "InstancedStruct.h"
#include "StructUtils/InstancedStruct.h"
#include "GlobalData.generated.h" // UHT	

// 데이터 테이블로 관리할 아이템 정보
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    // ── [공통 정보] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Info")
    FText ItemName{};

    // ── [공통 정보] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Info")
    FText ItemDescription{};

    // ── [공통 정보] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Info")
    EItemType ItemType{};

    // ── [공통 정보] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Info")
    int32 CurCount = 1;

    // ── [공통 정보] ── 인벤토리에서 겹쳐서 보관 할 수 있는 아이템인지.(true면 겹치기 가능)
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    //bool bIsStack;
    
    // ── [공통 정보] ── -1은 현재 아이템이 제대로 정의되어 있지 않은 상태라는 뜻
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Info")
    int32 MaxCount = -1; 

    // ── [비주얼 리소스 - 약참조 포인터] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual")
    TSoftObjectPtr<UTexture2D> IconTexture = nullptr;

    // ── [비주얼 리소스 - 약참조 포인터] ──
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSubclassOf<class AC_WeaponBase> WeaponClass;
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

    // ── [실시간 공통 데이터] ── 아이템의 갯수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 CurCount = 0;

    // ── [실시간 공통 데이터] ── 현재 사용?중인 플레이어의 ID, 현재는 드래그 드롭 중에 상호작용중인 PlayerID가 들어가고 있음.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 LockedByPlayerID = INDEX_NONE; // INDEX_NONE == -1
    
    // 어떤 구조체든 다형성처럼 동적으로 담을 수 있음
    // C++에서 값 생성하기     : CustomData = FInstancedStruct::Make(구조체);
    // C++에서 데이터 가져오기 : CustomData.GetPrt<구조체 타입>() or CustomData.Get<구조체 타입>() \
    // 초기화의 두가지 방법 : 
    // 1. CustomData.Reset(); : CustomData 자체를 비워버림.
    // 2. CustomData.InitializeAs<구조체 타입>(); : 특정 구조체 타입의 기본값으로 다시 생성할 때 사용.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FInstancedStruct CustomData;

public:
    // 빈 슬롯인지 확인하는 함수.
    bool IsEmpty() const {return ItemRowName.IsNone() || CurCount == 0;}
    
    // 비우는 함수, 멀티 환경 서버에서 사용시 MarkItemDirty(Entry)를 통해 변경을 알릴 것! 
    void Clear()
    {
        ItemRowName = NAME_None;
        CurCount = 0;
        LockedByPlayerID = INDEX_NONE; 
        SlotIndex = -1;
        CustomData.Reset();
    }
    
};

USTRUCT(BlueprintType)
struct FCursorItem
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FInventoryEntry Item{};           // 현재 들고 있는 아이템
    
    // TODO : 멀티 환경에서 InvenComponent를 포인터변수로 전송하는게 아니라 ID(int32)로 전송하는 방식으로 바꾸면 리팩토링 필요, 현재는 포인터 변수를 전송.
    //int32 SourceContainerID = -1;   // 상자의 ID 혹은 플레이어 인벤토리의 ID
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UC_InvenComponent* SourceInvenComp = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SourceSlotIndex   = -1;     // 몇 번째 슬롯이었나?
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsValid = false;            // 현재 커서에 아이템이 들려있는가?
    
    void Clear()
    {
        Item.Clear();
        SourceInvenComp = nullptr;
        SourceSlotIndex = -1;
        bIsValid = false;
    }
    
    // CursorItem을 세팅하는 함수.
    bool SetCursorItem(FInventoryEntry InEntry, UC_InvenComponent* InSourceInvenComp, int32 InSourceSlotIndex)
    {
        if (InEntry.ItemRowName == NAME_None || InSourceInvenComp == nullptr || InSourceSlotIndex == -1) return false;
        
        Item = InEntry;
        
        SourceInvenComp = InSourceInvenComp;
        
        SourceSlotIndex = InSourceSlotIndex;
        
        bIsValid = true;
        
        return true;
    }
};

// ******************************
// 아이템 CustomData 구조체 선언부
// ******************************
USTRUCT(BlueprintType)
struct FGunCustomData
{
    GENERATED_BODY()
    
    // 데미지의 업그레이드의 레벨 혹은 추가 데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Upgrade_Damage = 0;
    
    // MaxAmmo의 업그레이드의 레벨 혹은 추가 MaxAmmo
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Upgrade_MaxAmmo = 0;
    
    // FireRate의 업그레이드의 레벨 혹은 추가 FireRate
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Upgrade_FireRate = 0;
    
    // 이건 인벤이나 ItemPickUp에서 총의 CurAmmo값을 저장하기 위해 존재.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurAmmo = 0;
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

    // ── [총기 관련 스탯 (Stats)] ──
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

    // ── [총기 플레이어 애니메이션 (Player Animations)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player_Animations")
    TSoftObjectPtr<UAnimMontage> PlayerFireAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player_Animations")
    TSoftObjectPtr<UAnimMontage> PlayerReloadAnimation;
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


// ******************************
// Stat Data
// ******************************

/// <summary>
/// 공통 스탯 정보 (DataTable 용)
/// </summary>
USTRUCT(BlueprintType)
struct FCharacterStatData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
    float InitialMaxHP{};
};

/// <summary>
/// Player 전용 스탯 정보
/// </summary>
USTRUCT(BlueprintType)
struct FPlayerStatData : public FCharacterStatData
{
    GENERATED_BODY()
	
    // TODO : Player 쪽 사용할 Stat Data 추가해줄 것, 동시에 PlayerStatComponent에서 해당 값 참조해서 CurStatData들 초기화 처리해줄 것
};

/// <summary>
/// Monster 전용
/// </summary>
USTRUCT(BlueprintType)
struct FEnemyStatData : public FCharacterStatData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
    float	MoveSpeed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
    float	DetectRange;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
    float	LoseDetectRange;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
    float	Att;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
    float	Def;
	
};
