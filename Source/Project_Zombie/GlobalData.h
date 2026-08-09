#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GlobalEnum.h"
#include "Net/Serialization/FastArraySerializer.h"
//#include "InstancedStruct.h"
#include "StructUtils/InstancedStruct.h"
#include "GlobalData.generated.h" // UHT

// TODO : 강화 테이블 같은게 만들어져서 최대 강화 단계를 지정하면 그걸로 대체 하기. 그전까지는 이걸 사용.
#define MAX_GRADE 5 

// Key-Value 개별 항목
USTRUCT(BlueprintType)
struct FUpgradableKeyVal
{
    GENERATED_BODY()
    
	// 어떤 스탯을 업그레이드 할 것인지에 대한 키값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomData")
    EUpgradableStats Key = EUpgradableStats::None;

	// 업그레이드 등급 값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomData")
    uint8 Grade = 0; 
};

// FInstancedStruct 내부 데이터
USTRUCT(BlueprintType)
struct FUpgradableData
{
    GENERATED_BODY()

public:
    // 고정 장비 강화 정보 // TODO : 무기 티어를 사용한다면 필요하지만 아니라면 없애야 함.
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    //int32 EnhanceLevel = 0;

    // 실패 횟수인데 이게 꼭 필요할까? TODO : 강화 천장용인데 확률 강화를 사용할 것 인가?
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    //int32 FailPityCount = 0;

    // 동적 스탯 리스트 (TMap의 TArray 대안)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FUpgradableKeyVal> StatList{};

public:
    // ── [TMap 방식처럼 접근하기 위한 헬퍼 함수] ──

    // 1. 스탯 조회
    uint8 GetStatGrade(EUpgradableStats Key, uint8 DefaultValue = 0) const
    {
        for (const FUpgradableKeyVal& Pair : StatList)
        {
            if (Pair.Key == Key)
            {
                return Pair.Grade;
            }
        }
        return DefaultValue;
    }

    // 2. 스탯 설정
    void SetStatGrade(EUpgradableStats Key, uint8 NewValue)
    {
        for (FUpgradableKeyVal& Pair : StatList)
        {
            if (Pair.Key == Key)
            {
                Pair.Grade = NewValue;
                return;
            }
        }

        FUpgradableKeyVal NewPair;
        NewPair.Key = Key;
        NewPair.Grade = NewValue;
        StatList.Add(NewPair);
    }

    // 3. 스탯 누적
    void AddStatGrade(EUpgradableStats Key, uint8 AddValue)
    {
        uint8 CurrentVal = GetStatGrade(Key);
        SetStatGrade(Key, CurrentVal + AddValue);
    }
    
    //int32 GetStatList
};

// 데이터 테이블로 관리할 아이템 정보, 기본적으로는 C_ItemPickUp으로 스폰할 때 많이 사용.
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
    
    // ── [비주얼 리소스 - 포인터] ──
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual")
    //TObjectPtr<UTexture2D> IconTexture = nullptr;    

    // ── [비주얼 리소스 - 약참조 포인터] ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Visual")
    TSoftObjectPtr<UStaticMesh> DropMesh = nullptr;
    
    // 처음에 세팅해줄 수 있는 업그레이드 데이터를 담은 구조체.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FUpgradableData CustomData{};

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
	// Memo : 추후 확장성을 위해 FInstancedStruct를 사용하는 구조를 유지 중.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FInstancedStruct CustomData{};

public:

    // ── [CustomData 접근 헬퍼 함수] ──

	// CustomData가 FEquipmentCustomData 구조체를 담고 있는지 확인하는 함수.
    bool HasEquipmentData() const
    {
        return CustomData.GetScriptStruct() == FUpgradableData::StaticStruct();
    }

	// CustomData가 FEquipmentCustomData 구조체를 담고 있다면 해당 구조체의 포인터를 반환하는 함수.
    const FUpgradableData* GetEquipmentData() const
    {
        return CustomData.GetPtr<FUpgradableData>();
    }
    //GetOrCreateEquipmentData
	// CustomData가 FEquipmentCustomData 구조체를 담고 있지 않다면 새로 생성하고 해당 구조체의 포인터를 반환하는 함수.
    FUpgradableData* GetOrCreateEquipmentData()
    {
        if (!HasEquipmentData())
        {
            CustomData = FInstancedStruct::Make(FUpgradableData());
        }
        return CustomData.GetMutablePtr<FUpgradableData>();
    }

    FUpgradableData* GetEquipmentDataPtr()
    {
        return CustomData.GetMutablePtr<FUpgradableData>();
    }
    
    // 빈 슬롯인지 확인하는 함수.
    bool IsEmpty() const {return ItemRowName.IsNone() || CurCount == 0;}
    
    // 비우는 함수, 멀티 환경 서버에서 사용시 MarkItemDirty(Entry)를 통해 변경을 알릴 것! 
    // 그리고 <슬롯도 초기화>되므로 주의하기.
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


// ******************************
// 아이템 업그레이드 데이터 테이블 
// ******************************

// 1. 단일 재료 정보 (아이템 ID + 개수)
USTRUCT(BlueprintType)
struct FUpgradeMaterialInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MatterItemID = NAME_None; // 예: "Item_Gold", "Item_Iron"

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredCount = 0;
};

// 2. 특정 '단계(Grade)'로 강화할 때 필요한 재료 목록
USTRUCT(BlueprintType)
struct FGradeCostInfo
{
    GENERATED_BODY()

    // 예: 1강, 2강, 3강...
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TargetGrade = 0;

    // 해당 단계 강화 시 필요한 재료 배열
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FUpgradeMaterialInfo> RequiredMaterials;
};

// 3. 특정 '스탯(Stat)'의 강화 비용 목록 (1강~N강까지의 비용)
USTRUCT(BlueprintType)
struct FStatUpgradeCostInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EUpgradableStats StatType = EUpgradableStats::None;

    // 스탯별 강화 단계 데이터 (1강 비용, 2강 비용, 3강 비용...)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGradeCostInfo> GradeCosts;
};

// 4. 데이터 테이블의 최종 Row (RowName = ItemRowName 예: "AK47", "Pistol_Rare")
USTRUCT(BlueprintType)
struct FItemUpgradeCostRow : public FTableRowBase
{
    GENERATED_BODY()

    // 해당 아이템이 지원하는 각 스탯별 강화 비용 데이터들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    TArray<FStatUpgradeCostInfo> StatUpgradeCosts;
    
    const FStatUpgradeCostInfo* GetTargetStatUpCostInfo(EUpgradableStats TargetStat) const
    {
        for (const FStatUpgradeCostInfo& StatUpgradeCost : StatUpgradeCosts)
        {
            if (StatUpgradeCost.StatType == TargetStat)
            {
                return &StatUpgradeCost;
            }
        }
        return nullptr;
    }
};

// 무기의 강화가능 스탯과 Grade당 올라가는 Value
USTRUCT(BlueprintType)
struct FWeaponUpgradeData : public FTableRowBase
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EUpgradableStats, float> GradePerValue{};
    
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

    // ── [강화 공식 데이터] ──
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Rules")
    float DamagePerUpgradeLevel = 5.0f; // 레벨당 데미지 증가량

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Rules")
    float AttackRatePerUpgradeLevel = 0.0f;
    
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

    // ── [강화 공식 데이터] ──
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Rules")
    int32 MaxAmmoPerUpgradeLevel = 5; // 레벨당 최대 탄약 증가량

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


    // ── [강화 공식 데이터] ──
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Rules")
    float ExplosionRadiusPerUpgradeLevel = 5; // 레벨당 폭발 반경 증가량 // TODO : 폭발 이펙트 크기는 범위랑 어케 연결하지.

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Rules")
    float MaxDamagePerUpgradeLevel = 5.f; // 레벨당 최대 데미지 증가량

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade Rules")
    float MinDamagePerUpgradeLevel = 5.f; // 레벨당 최대 데미지 증가량
};

// 포션 데이터 테이블
USTRUCT(BlueprintType)
struct FPotionData : public FWeaponData
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion | Stats")
    float Value{};
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player_Animations")
    TSoftObjectPtr<UAnimMontage> UsingMontage{};
};

// ******************************
// 무기 데이터 테이블 구조체 선언부
// ******************************




// ******************************
// 몬스터 드롭 데이터 에셋 Entry
// ******************************
USTRUCT(BlueprintType)
struct FDropEntry
{
    GENERATED_BODY()

    // General DataTable의 ItemRowName
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
    FName ItemRowName = NAME_None;

    // 드랍 확률 (0.0 = 0%, 1.0 = 100%)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DropChance = 0.1f; // 기본 10%

    // 드랍 수량 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", meta = (ClampMin = "1"))
    int32 MinCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", meta = (ClampMin = "1"))
    int32 MaxCount = 1;
};




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

    // 최대 체력 : 강화를 통해 Max값 늘리기.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float				MaxHP{};

    // 현재 체력 : StatComp -> StatComponent에서 동적 생성해서 MaxHP로 초기화
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    //float				CurHP{}; 
};

/// <summary>
/// Player 전용 스탯 정보
/// </summary>
USTRUCT(BlueprintType)
struct FPlayerStatData : public FCharacterStatData
{
    GENERATED_BODY()
	
    // TODO : Player 쪽 사용할 Stat Data 추가해줄 것, 동시에 PlayerStatComponent에서 해당 값 참조해서 CurStatData들 초기화 처리해줄 것
    // => 여기서부터는 나중에 StatComponent으로 분리? -> 분리작업 실시
    // 기본 이동 속도 : StatComp
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    //float	BaseMaxSpeed{};

    // 달리기 속도 : 강화를 통해 Max값 늘리기.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float   SprintSpeed{};
    
    // 최대 부스트 : 강화를 통해 Max값 늘리기.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float	MaxBoost{};

    // 현재 부스트 : StatComp -> StatComponent에서 동적 생성, MaxBoost로 초기화.
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    //float	CurBoost{};

    // 달리기 중 초당 부스트 소모량 : 강화를 통해 부스트 소모량 줄이기.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float   SprintBoostUseCost{};

    // 달리지 않을 때 초당 부스트 회복량 : 강화를 통해 부스트 회복량 늘리기.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float   BoostRecoverCost{};

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


namespace Helper
{
    inline FText GetStatDisplayName(EUpgradableStats StatType)
    {
        const UEnum* EnumPtr = StaticEnum<EUpgradableStats>();
        return EnumPtr ? EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(StatType)) : FText::GetEmpty();
    }
}


// StatComponent에서 m_Stat에서 오타를 줄이기 위해 사용.
namespace StatName
{
    const FName MaxHP        = TEXT("MaxHP");
    const FName CurHP        = TEXT("CurHP");
    const FName SprintSpeed  = TEXT("SprintSpeed");
    const FName MaxBoost     = TEXT("MaxBoost");
    const FName CurBoost     = TEXT("CurBoost");
    const FName BoostCost    = TEXT("SprintBoostUseCost");
    const FName BoostRecover = TEXT("BoostRecoverCost");
    // 필요한 스탯 추가해서 사용하면 됨.
    // StatName::MaxHP == TEXT("MaxHP")
}

// Player의 강화가능 스탯과 Grade당 올라가는 Value
// 스탯이름을 RowName으로 사용하고, 단계마다 필요한 재료 목록을 넣어주자.
USTRUCT(BlueprintType)
struct FPlayerStatUpgradeData : public FTableRowBase
{
    GENERATED_BODY()
    
    // 단계별 요구 재료
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGradeCostInfo> GradeCost{};
    
    // 단계별 상승 값
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> GradeValue{};    
};

enum class EZombieType : uint8;

// 좀비 타입마다 설정해줄 스폰값
USTRUCT(BlueprintType)
struct FZombieTypeSpawnSetting
{
    GENERATED_BODY()

public:
    // 스폰할 좀비 타입
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EZombieType ZombieType{};

    // 다른 타입과 비교한 스폰 선택 가중치
    // 숫자가 높을수록 선택될 가능성이 높음
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float SpawnWeight = 1.f;

    // 해당 타입 스폰 쿨타임
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float SpawnCoolDown = 0.f;

    // 해당 타입이 필드에 동시에 존재할 수 있는 최대 수
    // 0이면 타입별 제한 x
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
    int32 MaxActiveCount = 1;
};

// 좀비 웨이브마다 설정해줄 값
USTRUCT(BlueprintType)
struct FZombieWaveSetting
{
    GENERATED_BODY()

public:
    // 좀비 스폰 시도 간격
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.1"))
    float SpawnInterval = 2.f;

    // 한번에 Spawn Tick에서 꺼낼 좀비 수 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
    int32 SpawnCountPerTick = 1;

    // 웨이브에서 등장 가능한 타입별 설정
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FZombieTypeSpawnSetting> ZombieTypeSetting{};
};

USTRUCT(BlueprintType)
struct FStatSyncPair
{
    GENERATED_BODY()

    UPROPERTY()
    FName StatName{};

    UPROPERTY()
    float StatValue{};

    UPROPERTY()
    uint8 StatGrade{};
};