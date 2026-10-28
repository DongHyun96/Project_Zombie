// Fill out your copyright notice in the Description page of Project Settings.


#include "C_FaceToLocalPlayerComponent.h"

#include "GameModeAndManager/C_UIManager.h"
#include "GameModeAndManager/GameLevelManager/C_GameLevelManager.h"

UC_FaceToLocalPlayerComponent::UC_FaceToLocalPlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UC_FaceToLocalPlayerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UC_FaceToLocalPlayerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!m_EnableRotation) return;
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->PlayerCameraManager) return;

	FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
	CameraRotation.Pitch    = -CameraRotation.Pitch;
	CameraRotation.Yaw      += 180.f;
	
	for (auto It = m_RotatingSceneComponents.CreateIterator(); It; ++It)
	{
		USceneComponent* SceneCom = *It;
		if (!SceneCom) It.RemoveCurrent(); // 만일 플레이 중 삭제되어버린 SceneCom(웬만하면 없을 거 같긴한데) 예외처리 포함
		
		SceneCom->SetWorldRotation(CameraRotation);					
	}
}

void UC_FaceToLocalPlayerComponent::RegisterSceneComToRotate(USceneComponent* _SceneComponent)
{
	if (!_SceneComponent)
	{
		PRINT_LOCAL(GetWorld(), "[UC_FaceToLocalPlayerComponent::RegisterSceneComToRotate] : Received SceneCom nullptr", FColor::Red, 10.f);
		return;
	}
	if (m_RotatingSceneComponents.Contains(_SceneComponent)) return;
	
	PRINT_LOCAL(GetWorld(), "[UC_FaceToLocalPlayerComponent::RegisterSceneComToRotate]", FColor::Red, 10.f);
	m_RotatingSceneComponents.Add(_SceneComponent);
}
