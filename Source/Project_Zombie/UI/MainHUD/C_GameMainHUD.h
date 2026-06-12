// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Actor/ItemActor/Weapon/Gun/C_GunBase.h"
#include "C_GameMainHUD.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ZOMBIE_API UC_GameMainHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public: // HP Bar 및 BoostBar 정보 업데이트 관련

	/// <summary>
	/// HP Bar Percent 업데이트 
	/// </summary>
	/// <param name="_HP"> : 현재 체력 </param>
	/// <param name="_MaxHP"> : 최대 체력 값 </param>
	/// <returns> : 잘못된 값이 들어왔을 경우 return false </returns>
	UFUNCTION(BlueprintCallable)
	bool UpdateHPBar(float _HP, float _MaxHP);

	/// <summary>
	/// Boost Bar 업데이트
	/// </summary>
	/// <param name="_Boost"> : 현재 Boost 량 </param>
	/// <param name="_MaxBoost"> : 최대 Boost 량 </param>
	/// <returns> : 잘못된 값이 들어왔을 경우 return false </returns>
	UFUNCTION(BlueprintCallable)
	bool UpdateBoostBar(float _Boost, float _MaxBoost);

public: // 무기 상태 정보 업데이트 관련
	
	/// <summary>
	/// AmmoInfo 정보 Visibility 켜기/끄기 
	/// </summary>
	/// <param name="_Visible"> : 켜기/끄기 옵션 </param>
	/// <param name="_FireMode"> : 처음 표기할 FireMode / 켜기 옵션 시 default parameter 쓰지말고 해당 FireMode 넣어줄 것 </param>
	/// <param name="_MagazineAmmo"> : 처음 표기할 탄창에 남은 장탄수 / 켜기 옵션 시 default parameter 쓰지말고 해당 FireMode 넣어줄 것 </param>
	/// <param name="_LeftAmmoTotalCount"> : 처음 표기할 해당 총기 Type의 전체 보유한 탄약 수 / 켜기 옵션 시 default parameter 쓰지말고 해당 FireMode 넣어줄 것 </param>
	/// <returns> : 잘못된 인자값이 들어온 경우, 처리되지 않고 return false </returns>
	bool ToggleAmmoInfoVisibility
	(
		bool		_Visible,
		EFireMode	_FireMode 		= EFireMode::End,
		int32		_MagazineAmmo 	= 0,
		int32		_LeftAmmoTotalCount	= 0
	);
	
	/// <summary>
	/// <para> 현재 탄창에 장착된 장탄수 업데이트 </para>
	/// <para> 주의 : ToggleAmmoInfoVisibility(_Visible true) 로 먼저 AmmoInfo 보여주고(무기를 꺼내는 등의 처리에서 호출이 되어야 함) </para>
	/// <para> -> 무기를 발사할 때, 장탄수 업데이트 시, 이 함수 이용할 것 </para>
	/// </summary>
	/// <param name="_AmmoCount"> : 탄창 장탄수 </param>
	void UpdateMagazineAmmoCount(int32 _AmmoCount);

	/// <summary>
	/// <para> 현재 탄창에 장착된 장탄수 업데이트 </para>
	/// <para> 주의 : ToggleAmmoInfoVisibility(_Visible true) 로 먼저 AmmoInfo 보여주고(무기를 꺼내는 등의 처리에서 호출이 되어야 함) </para>
	/// <para> -> 무기를 발사할 때, 장탄수 업데이트 시, 이 함수 이용할 것 </para>
	/// </summary>
	/// <param name="_LeftAmmoTotalCount"> : 해당 무기의 사용할 수 있는 총 탄약 수 </param>
	void UpdateLeftAmmoTotalCount(int32 _LeftAmmoTotalCount);
	
protected:

	UPROPERTY(meta = (BindWidget))
	class UC_PlayerStatWidget* PlayerStatWidget{};

};
