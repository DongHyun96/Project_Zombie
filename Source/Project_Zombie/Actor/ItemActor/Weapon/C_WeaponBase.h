// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h" // 이건 뭐지?
//#include "GameFramework/Actor.h"
//#include "Actor/ItemActor/C_ItemActor.h"
#include "GameFramework/Actor.h"
#include "C_WeaponBase.generated.h"

struct FAmmoUIInfo;
struct FStreamableHandle;
struct FWeaponData;
class UC_ItemLinkComponent;

UCLASS(Abstract)
class PROJECT_ZOMBIE_API AC_WeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AC_WeaponBase();

protected:
	virtual void BeginPlay() override;

	// 액터가 레벨에서 제거되거나 Destroy될 때 호출되는 언리얼 기본 이벤트 함수, 이 때 비동기 로드중인 에셋 취소
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:	
	virtual void Tick(float DeltaTime) override;

	// 아이템 매니저에서 스폰될 때 초기화 시켜주는 함수.
	// 원래는 더 큰 구조로 잡아서 ItemActor로 모든 기능성 아이템을 포괄하고
	// const void*로 모든 데이터 테이블을 받아오려다가 우선 무기 한정으로 바꿈. -> TODO : 모든 아이템 데이터 테이블은 FItemData를 상속받으면?
	// 데이터 테이블의 Base Data + FInventoryEntry의 동적 데이터를 모두 업데이트 해야 한다.
	// 데이터 테이블은 아이템 매니저에서 매개변수로 주고, FInventoryEntry는 ItemLink를 통한다. 
	// 실제 자신의 클래스까지 내려가서 재정의해야 한다.
	virtual bool InitializeItemActor(const FWeaponData* InRawData) PURE_VIRTUAL(AC_WeaponBase::InitializeItemActor, return false;);

	virtual void InitializeItemData(const FWeaponData* InRawData) PURE_VIRTUAL(AC_WeaponBase::InitializeItemData, );

	// 클라이언트가 생성된 Weapon의 에셋들을 로드하기 위한 리플리케이션 함수.
	// TODO 만약 이걸로 이 무기의 Owner가 아닌 클라들이 에셋을 로드하지 못한다면 멀티캐스트로 다 호출해줘야함.
	UFUNCTION()
	void OnRep_WeaponRowName();
	
protected:

	// 데이터 테이블의 에셋들을 비동기 로드하기 위한 함수, 무기마다 다를 수 있기 때문에 순수 가상 함수로 선언. return 값을 bool 처리 할까?
	virtual void LoadAsyncAssets(const FWeaponData* InRawData) PURE_VIRTUAL(AC_WeaponBase::LoadAsyncAssets, );
	
	void CancelAsyncLoad();
public:
	
	/// <summary>
	/// 발사 시작 동작 처리 (기본 키 : LMB Started (발사 키 클릭 이벤트 발생 시))
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : StartFire 처리가 필요없거나(이건 웬만한 무기는 다 필요할 듯), 모종의 이유로 실패했을 경우 return false </returns>
	virtual bool OnStartFire(class AC_BasicPlayer* _WeaponUser) PURE_VIRTUAL(AC_WeaponBase::OnStartFire, return false;);

	/// <summary>
	/// 발사 동작 지속동작 처리 (기본 키 : LMB OnGoing (계속 누르고 있는 시점) )
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : FireOnGoing 처리가 필요없거나 모종의 이유로 실패했을 경우 return false </returns>
	virtual bool OnFireOnGoing(AC_BasicPlayer* _WeaponUser) { return false; }

	/// <summary>
	/// 발사 끝났을 때 처리 (기본 키 : LMB Completed (발사키 떼었을 때 시점) )
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : FireEnd 처리가 필요없거나 모종의 이유로 실패했을 경우 return false </returns>
	virtual bool OnFireEnd(AC_BasicPlayer* _WeaponUser) { return false; }

	/// <summary>
	/// 사격 모드 전환 처리 (기본 키 : B키 기능 )
	/// </summary>
	virtual void SwitchFireMode() { return ; };

	/// <summary>
	/// Reload 키 동작 처리 (기본 키 R키 기능) 
	/// </summary>
	/// <param name="_WeaponUser"> : 이 Weapon을 사용하는 Player 객체 </param>
	/// <returns> : R키에 대한 처리가 필요없거나 실패했을 경우 return false </returns>
	virtual bool Reload(AC_BasicPlayer* _WeaponUser) { return false; }

	/*virtual void StartAttack(class AC_BasicPlayer* _WeaponUser) PURE_VIRTUAL(AC_WeaponBase::StartAttack, );
	virtual void StopAttack(AC_BasicPlayer* _WeaponUser) PURE_VIRTUAL(AC_WeaponBase::StopAttack, );*/

	/// <summary>
	/// 무기집에 무기 붙이기
	/// </summary>
	/// <returns> 실패 시 return false </returns>
	virtual bool AttachToHolster(USceneComponent* _ParentMesh) PURE_VIRTUAL(AC_WeaponBase::AttachToHolster, return false;);

	/// <summary>
	/// Player 손에 장착하기
	/// </summary>
	/// <returns> 실패 시 return false </returns>
	virtual bool AttachToHand(USceneComponent* _ParentMesh) PURE_VIRTUAL(AC_WeaponBase::AttachToHand, return false;);

public:
	
	/// <summary>
	/// 무기 집어넣기 처리 시 초기화할 부분 초기화 (TODO : 필요하다면 override 처리할 것) 
	/// </summary>
	virtual void OnSheathStart() {}
	
public:
	
	/// <summary>
	/// 현재 무기 상태에 맞춘 AmmoUIInfo 초기화 처리 
	/// </summary>
	/// <param name="_AmmoUIInfo"></param>
	virtual void SetAmmoUIInfo(FAmmoUIInfo& _AmmoUIInfo) PURE_VIRTUAL(AC_WeaponBase::SetAmmoUIInfo);
	
	/// <summary>
	/// <para> DrawEnd 시, 각 무기에 맞는 표기할 무기 정보 띄우기 처리 (Local Player에 한해 처리) </para>
	/// <para> 이 무기의 주인이 Locally Controlled 되는 중인지 체킹하여 valid하면 띄움 </para>
	/// <para> 실질적인 DrawEnd 시에 호출 및 리슨서버 환경에서 EquippedCom에서 현재 들고 있는 무기가 바뀌는 Rep 처리 시, </para>
	/// <para> 해당 함수를 이용할 예정 </para>
	/// </summary>
	virtual void UpdateAmmoInfoHUDForDrawEnd() PURE_VIRTUAL(AC_WeaponBase::UpdateAmmoInformUIOnDrawEnd, );

	void SetRelativeTransformToInitial() { SetActorRelativeTransform(m_InitialRelativeTransform); }
	
public:
	
	UAnimMontage* GetDrawMontage() const { return m_DrawMontage; }
	UAnimMontage* GetSheathMontage() const { return m_SheathMontage; }

	
	FName GetWeaponRowName() {return m_WeaponRowName;}
public:
	
	AC_BasicPlayer* GetOwnerPlayer() const { return m_OwnerPlayer; }
	void SetOwnerPlayer(AC_BasicPlayer* _OwnerPlayer) { m_OwnerPlayer = _OwnerPlayer; }
	
	UC_ItemLinkComponent* GetLinkComp() {return ItemLinkComp;}

	//
	void SetItemRowName(FName InRowName) { m_WeaponRowName = InRowName; }
	
protected:

	// 무기의 고유 RowName (서버에서 설정되면 클라이언트로 복제됨)
	UPROPERTY(ReplicatedUsing = OnRep_WeaponRowName, Transient)
	FName m_WeaponRowName{};

public:
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	UPROPERTY(Replicated)
	AC_BasicPlayer* m_OwnerPlayer{};
	


protected:
	// TODO : 이 두 애니메이션도 데이터 테이블에 넣어주는게 나을 것 같긴한데.
	// Draw, Sheath는 블프에서 넣어주고 있음.
	// TODO : 동작이 고정된 되어서 사용한다면 플레이어쪽에, 무기마다 다르다면 데이터 테이블로 처리하는게 나을 듯
	
	// 해당 무기의 무기 꺼내는 동작 Montage (Player character montage)
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (DisplayName = "DrawMontage"))
	UAnimMontage* m_DrawMontage{};
	
	// 해당 무기의 무기집에 집어넣는 동작 Montage (Player character montage)
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (DisplayName = "SheathMontage"))
	UAnimMontage* m_SheathMontage{};

	// 데이터 연동 전용 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UC_ItemLinkComponent> ItemLinkComp{};
	
protected:
	// 비동기 로딩 핸들을 관리할 스마트 포인터
	TSharedPtr<FStreamableHandle> m_AsyncLoadHandle;
	
private:

	// 초기 RelativeTransform 값 (부착 시 해당값으로 초기화 처리가 들어가야 제대로 된 위치가 잡힘)
	FTransform m_InitialRelativeTransform{};
	
};
