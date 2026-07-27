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
    
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    //TSubclassOf<class AC_WeaponBase> WeaponClass;
    
    // ── [실제 기능 액터 클래스] ──
    // 무기일 수도 있고, 나중에 설치형 가짓/특수 장비일 수도 있음 (AActor 상속)
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Logic")
    //TSubclassOf<AActor> EquippedActorClass;
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
    
    // EItemType을 FInventoryEntry에서도 들고 있을 것 인가? 
    
    // 어떤 구조체든 다형성처럼 동적으로 담을 수 있음
    // C++에서 값 생성하기     : CustomData = FInstancedStruct::Make(구조체);
    // C++에서 데이터 가져오기 : CustomData.GetPrt<구조체 타입>() or CustomData.Get<구조체 타입>() \
    // 초기화의 두가지 방법 : 
    // 1. CustomData.Reset(); : CustomData 자체를 비워버림.
    // 2. CustomData.InitializeAs<구조체 타입>(); : 특정 구조체 타입의 기본값으로 다시 생성할 때 사용.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FInstancedStruct CustomData{};

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
    EWeaponType WeaponType = EWeaponType::Gun;

    // ── [무기 공통 스탯 (Stats)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float BaseDamage = 1.f;      // 무기 기본 데미지

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRate = 0.1f;     // 총기 발사간격, 근접 공속, 투척 딜레이 공통 사용 (시간 간격 : 0.1초 = 초당 10발)

    // ── [무기 공통 매쉬 (Mesh)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<USkeletalMesh> WeaponSkeletalMesh{};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UStaticMesh> WeaponStaticMesh{};
    
    // ── [무기 클래스 지정] ──
    // 무기일 수도 있고, 나중에 설치형 가짓/특수 장비일 수도 있음 (AActor 상속)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Logic")
    TSubclassOf<AActor> EquippedActorClass{};
};

// 데이터 테이블로 관리할 총기 데이터
USTRUCT(BlueprintType)
struct FGunData : public FWeaponData
{
    GENERATED_BODY()

    // ── [총기 관련 스탯 (Stats)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MaxAmmo = 7;     // 총기 최대 탄창 수

    // ── [총기 부가 설정] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ShellEjectImpulse = 150.0f; // 탄피 배출에 가하는 힘.

    // ── [총기 관련 매쉬 (Mesh)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UStaticMesh> ShellMesh{};   // 총기 탄피 매쉬

    // ── [총기 애니메이션 (Animations)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TSoftObjectPtr<UAnimSequence> FireAnimation{};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TSoftObjectPtr<UAnimSequence> ReloadAnimation{};

    // ── [총기 플레이어 애니메이션 (Player Animations)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player_Animations")
    TSoftObjectPtr<UAnimMontage> PlayerFireAnimation{};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player_Animations")
    TSoftObjectPtr<UAnimMontage> PlayerReloadAnimation{};
};

// 근접무기 데이터
USTRUCT(BlueprintType)
struct FMeleeData : public FWeaponData
{
    GENERATED_BODY()

    // ── [플레이어 근접무기 애니메이션 (Animations)] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TSoftObjectPtr<UAnimMontage> PlayerAttackAnimation{};
};

// 투척류 데이터
USTRUCT(BlueprintType)
struct FThrowableData : public FWeaponData
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UAnimMontage> m_ThrowMontage{};
    
    // 핀 제거 가능 여부 
    // 핀 제거 동작 몽타주를 넣을 것인가?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable|State")
    bool m_bHasPin = false;

    // 쿠킹 가능 여부 
    // R키를 눌렀을 때 쿠킹 가능한가?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable|State")
    bool m_bIsCookable = false;

    // 충돌 시 폭발 여부
    // 충돌하면 바로 폭발하는가?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable|State")
    bool m_bExplodeOnImpact = false;

    // 폭발까지 걸리는 시간 (핀 제거 후, 폭발까지 걸리는 시간)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable|State",
        meta = (EditCondition = "!m_bExplodeOnImpact", EditConditionHides, ClampMin = "0.0"))
    float m_FuseTime = 0.f;

    // 투척 속도
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable|State")
    //float m_ThrowSpeed;
    
    // 전략 패턴 : 폭발 기능 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable|Launch")
    TSoftClassPtr<UObject> m_ExplodeStrategyClass{};
    
    // 장판 데미지 영역 클래스
    // 일단 화염병 전용으로 AC_FireDamageArea를 사용하지만, 나중에 다른 장판 데미지 영역이 생기면 수정 예정	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Throwable|Fire Damage Area")
    // TODO : 전방선언으로 우선 사용하는데 문제가 생기면 그냥 빼버리기
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable|Damage Area")
    TSoftClassPtr<class AC_FireDamageArea> m_FireDamageAreaClass{};
    
    // 폭발 반경
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable|Explosion")
    float m_ExplosionRadius = 0.0f;

    // 최대 데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable|Explosion")
    float m_MaxDamage = 0.0f;

    // 최소 데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable|Explosion")
    float m_MinDamage = 0.0f;
    
    // 폭발 이펙트 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable|Effect")
    TSoftObjectPtr<UParticleSystem> m_ExplosionEffect{};

    // 폭발 이펙트 크기 (1.0 = 기본 크기)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable|Effect")
    float m_ExplosionEffectScale = 1.0f;
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
    float	MoveSpeed{};

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
    float	DetectRange{};

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
    float	LoseDetectRange{};

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
    float	Att{};

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
    float	Def{};
	
};
