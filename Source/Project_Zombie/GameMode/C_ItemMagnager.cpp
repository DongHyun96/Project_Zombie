// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/C_ItemMagnager.h"

void UC_ItemMagnager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 생성자나 에디터 지정이 아닌, 코드에서 데이터 테이블을 static load 합니다.
    // 경로(Path)는 본인의 데이터 테이블 에디터 경로로 수정하세요.
    //static ConstructorHelpers::FObjectFinder<UDataTable> DataTableFinder(TEXT("/Game/Blueprints/Data/DT_ItemData_Sang.DT_ItemData_Sang"));
}

const FItemData* UC_ItemMagnager::GetItemData(FName InRowName) const
{
    if (!ItemDataTable || InRowName.IsNone()) return nullptr;

    // FindRow는 내부적으로 TMap 해시 조회를 하기 때문에 매우 빠릅니다.
    return ItemDataTable->FindRow<FItemData>(InRowName, TEXT("GetItemDataContext"));
}
