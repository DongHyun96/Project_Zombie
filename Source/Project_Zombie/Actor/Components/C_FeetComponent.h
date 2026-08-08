#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_FeetComponent.generated.h"

class AC_BasicPlayer;
class USoundAttenuation;
class USoundConcurrency;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_ZOMBIE_API UC_FeetComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UC_FeetComponent();

protected:
	virtual void BeginPlay() override;

public:

	/// <summary>
	/// 발자국 소리 재생
	/// </summary>
	/// <param name="_IsLeftFoot">왼쪽 발일 경우 true</param>
	void PlayFootstep(bool _IsLeftFoot);

	/// <summary>
	/// 착지 소리 재생
	/// </summary>
	void PlayLandingSound(const FHitResult& _Hit);

private:

	UPROPERTY()
	TObjectPtr<AC_BasicPlayer> m_OwnerPlayer{};

	UPROPERTY(EditAnywhere, Category = "Footstep|Socekt")
	FName m_LeftFootSocketName;
	
	UPROPERTY(EditAnywhere, Category = "Footstep|Socekt")
	FName m_RightFootSocketName;

	UPROPERTY(EditAnywhere, Category = "Footstep|Volume")
	float m_LocalVolume;

	UPROPERTY(EditAnywhere, Category = "Footstep|Volume")
	float m_RemoteVolume;
	
	UPROPERTY(EditAnywhere, Category = "Footstep|Volume")
	float m_CrouchPitch;

	UPROPERTY(EditAnywhere, Category = "Footstep|Sound")
	TObjectPtr<USoundAttenuation> m_FootstepAttenuation;

	UPROPERTY(EditAnywhere, Category = "Footstep|Sound")
	TObjectPtr<USoundConcurrency> m_FootstepConcurrency;
};
