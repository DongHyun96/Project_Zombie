// Fill out your copyright notice in the Description page of Project Settings.


#include "C_WeaponDataTableComponent.h"

UC_WeaponDataTableComponent::UC_WeaponDataTableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UC_WeaponDataTableComponent::InitWeaponDataFromStruct(UScriptStruct* _InStruct, const void* _StrctPtr)
{
	if (nullptr == _InStruct || nullptr == _StrctPtr)
		return;

	// 구조체 정보를 순회하면서 맴버 데이터를 확인한다.
	for (TFieldIterator<FProperty> Iter(_InStruct); Iter; ++Iter)
	{
		FProperty* Property = *Iter;

		// 맴버변수 이름
		FName StatName = Property->GetFName();

		// FLoat 타입 맴버만 필터링
		if (FFloatProperty* FloatPro = CastField<FFloatProperty>(Property))
		{
			float Value = FloatPro->GetPropertyValue_InContainer(_StrctPtr);
			AddData(StatName, Value);
		}
		// FSoftObject 타입 맴버만 필터링
		else if (FSoftObjectProperty* SoftObjPro = CastField<FSoftObjectProperty>(Property))
		{
			const FSoftObjectPtr& SoftPtr = *SoftObjPro->GetPropertyValuePtr_InContainer(_StrctPtr);
			FSoftObjectPath AssetPath = SoftPtr.ToSoftObjectPath();
			TSoftObjectPtr<UObject> SafeSoftPtr(AssetPath);

			// 소프트 포인터를 전달.
			AddAssetData(StatName, SafeSoftPtr);
		}
	}
}


