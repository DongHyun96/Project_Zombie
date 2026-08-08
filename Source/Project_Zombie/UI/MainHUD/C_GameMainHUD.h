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
	/// <param name="_LeftAmmoTotalCount"> : 처음 표기할 해당 총기 Type의 전체 보유한 탄약 수(or 한탄창 탄약 수 -> 추후 기획에 따라 달라짐) / 켜기 옵션 시 default parameter 쓰지말고 해당 FireMode 넣어줄 것 </param>
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
	
	/// <summary>
	/// FireMode 변경 
	/// </summary>
	/// <returns> : 현재 AmmoInfo Visibility false이거나 새로 들어온 FireMode가 이미 보여지는 FireMode인 경우, Update 하지 않음(return false) </returns>
	bool UpdateFireMode(EFireMode _NewFireMode);

public: // Interaction UI 관련

	/// <summary>
	/// 상호작용 UI 활성화
	/// </summary>
	/// <param name="_InteractionText">띄어줄 Text</param>
	void ActivateInteractionUI(const FText& _InteractionText);

	/// <summary>
	/// 상호작용 UI 비활성화
	/// </summary>
	void DeactivateInteractionUI();

	/// <summary>
	/// 상호작용 시 Timer 활성화
	/// </summary>
	/// <param name="_Duration">상호작용 시간</param>
	void ActivateInteractionTimer(float _Duration);

	/// <summary>
	/// 상호작용 종료 시 Timer 비활성화
	/// </summary>
	void DeactivateInteractionTimer();


public:
	
	class UC_CompassBarWidget* GetCompassBarWidget() const { return CompassBarWidget; }
	
	class UC_CrosshairWidget* GetCrosshairWidget() const { return CrosshairWidget; }
	
	class UC_PlayerStatWidget* GetPlayerStatWidget() const { return PlayerStatWidget; }

	class UC_InteractionWidget* GetInteractionWidget() const { return InteractionWidget; }
	
	class UC_OtherPlayerStatWidget* GetOtherPlayerStatWidget() const { return OtherPlayerStatWidget; }

	class UC_GameOverWidget* GetGameOverWidget() const { return GameOverWidget; }
	
	class UC_InformWidget* GetInformWidget() const { return InformWidget; }
	
public: // Ingame Log 관련
	
	/// <summary>
	/// Player Warning Log 추가
	/// </summary>
	/// <param name="WarningLog"> : Warning log </param>
	/// <param name="_LogColor"> : Log 색상 </param>
	/// <returns> : 제대로 추가되지 않았다면 return false </returns>
	UFUNCTION(BlueprintCallable)
	bool AddPlayerWarningLog(const FString& WarningLog, const FColor& _LogColor = FColor::White);

protected:

	UPROPERTY(meta = (BindWidget))
	UC_PlayerStatWidget* PlayerStatWidget{};
	
	UPROPERTY(meta = (BindWidget))
	class UC_InformWidget* InformWidget{};

	UPROPERTY(meta = (BindWidget))
	class UC_CompassBarWidget* CompassBarWidget{};

	UPROPERTY(meta = (BindWidget))
	class UC_CrosshairWidget* CrosshairWidget{};

	UPROPERTY(meta = (BindWidget))
	class UC_InteractionWidget* InteractionWidget{};

	UPROPERTY(meta = (BindWidget))
	UC_OtherPlayerStatWidget* OtherPlayerStatWidget{};
	
	UPROPERTY(meta = (BindWidget))
	UC_GameOverWidget* GameOverWidget{};
};
