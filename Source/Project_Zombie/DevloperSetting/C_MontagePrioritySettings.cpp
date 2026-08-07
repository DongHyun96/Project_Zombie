// Fill out your copyright notice in the Description page of Project Settings.


#include "C_MontagePrioritySettings.h"

#include "GameplayTagsModule.h"
#include "GameplayTagsManager.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(MONTAGE_PRIORITY)

UC_MontagePrioritySettings::UC_MontagePrioritySettings()
{
    CategoryName = TEXT("Game");
    SectionName  = TEXT("MontagePrioritySettings");
}

//FText UC_MontagePrioritySettings::GetSectionDescription() const
//{
//    return FText::FromString
//    (
//        TEXT
//        (
//            "몽타주 Priority 카테고리 '추가/삭제' 는 GameplayTags 탭에서 수정할 것\n0-255 까지의 우선순위 부여 가능 (더 높은 숫자가 우선적으로 재생처리)"
//        )
//    );
//}

#if WITH_EDITOR

void UC_MontagePrioritySettings::PostInitProperties()
{
    Super::PostInitProperties();

    if (GIsEditor) FCoreDelegates::OnPostEngineInit.AddUObject(this, &UC_MontagePrioritySettings::OnEngineInitComplete);
}

void UC_MontagePrioritySettings::OnEngineInitComplete()
{
    IGameplayTagsModule* TagsModule = FModuleManager::Get().LoadModulePtr<IGameplayTagsModule>(TEXT("GameplayTags"));
    if (!TagsModule)
    {
        UE_LOG(LogTemp, Warning, TEXT("UC_MontagePrioritySettings::OnEngineInitComplete : Failed to load GameplayTags module"));
        return;
    }
    
    SyncMontagePriorityTags();
    TagsModule->OnGameplayTagTreeChanged.AddUObject(this, &UC_MontagePrioritySettings::SyncMontagePriorityTags);
}

void UC_MontagePrioritySettings::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);

    if (!PropertyChangedEvent.Property) return;
    
    const FName PropertyName = PropertyChangedEvent.Property->GetFName();

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UC_MontagePrioritySettings, PriorityMap))
    {
        // 만약 사용자가 삭제를 시도 (삭제 키 자체는 없앨 수 없었음 -> 다시 원상태 복구 처리로 해둠)
        if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayRemove || 
            PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayClear)
        {
            UE_LOG(MONTAGE_PRIORITY, Warning, TEXT("요소 삭제가 차단되었습니다. 마지막 저장 상태로 롤백합니다."));
            
            // 메모리 복사본을 쓰는 대신, 가장 마지막에 SaveConfig() 되었던 정상 상태를 ini 파일에서 특정 프로퍼티만 읽어와서 덮어씌움
            FProperty* MapProp = GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UC_MontagePrioritySettings, PriorityMap));
            ReloadConfig(GetClass(), nullptr, UE::LCPF_None, MapProp);
            return;
        }
    }
        
    // 삭제가 아닌 단순 숫자(Priority) 변경일 경우, 실제로 Sync
    SyncMontagePriorityTags();
}

void UC_MontagePrioritySettings::SyncMontagePriorityTags()
{
    if (!IGameplayTagsModule::IsAvailable()) return;

    const FGameplayTag RootTag = FGameplayTag::RequestGameplayTag(TEXT("AnimMontagePriority"), false);
    if (!RootTag.IsValid()) return;

    const UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
    const FGameplayTagContainer ChildTags = TagsManager.RequestGameplayTagChildren(RootTag);

    TMap<FGameplayTag, uint8> UpdatedMap{};
    bool bIsMapChanged = false;

    for (const FGameplayTag& Tag : ChildTags)
    {
        if (PriorityMap.Contains(Tag))
        {
            UpdatedMap.Add(Tag, PriorityMap[Tag]);
        }
        else
        {
            UpdatedMap.Add(Tag, 0);
            bIsMapChanged = true;
        }
    }

    if (bIsMapChanged || UpdatedMap.Num() != PriorityMap.Num())
    {
        PriorityMap = UpdatedMap;
        
        // 정상적인 변경이 있을 때마다 엔진 내부적으로 ini 파일에 변경점을 기록함
        SaveConfig();
    }
}

#endif // WITH_EDITOR

bool UC_MontagePrioritySettings::GetPriority(const FGameplayTag& _MontagePriorityTag, uint8& _OutPriorityValue) const
{
    _OutPriorityValue = 0;
    
    if (const uint8* PriorityPtr = PriorityMap.Find(_MontagePriorityTag))
    {
        _OutPriorityValue = *PriorityPtr;
        return true;
    }
    
    return false;
}
