// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "C_SessionMgr.generated.h"

USTRUCT(BlueprintType)
struct FSessionData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	FString HostName;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayerNum;

	UPROPERTY(BlueprintReadOnly)
	int32 CurPlayerNum;

	UPROPERTY(BlueprintReadOnly)
	int32 Ping;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionOperationComplete, bool, bWasSuccessful);

UCLASS()
class PROJECT_ZOMBIE_API UC_SessionMgr : public UGameInstanceSubsystem
{
	GENERATED_BODY()
protected:
	// 언리얼 OSS 최고 관리자 IOnlineSubsystem
	// 방 생성, 찾기, 합류 등등 IOnlineSession;
	// 로그인정보, 계정인증	IOnlineIdentity;
	// 친구목록 탐색			IOnlineFriends;

	IOnlineSessionPtr					m_SessionInterface;		// 세션관련 매니저
	TSharedPtr<FOnlineSessionSearch>	m_SesionSearchResult;	// 탐색을 하면 만들어지는 객체, 탐색된 유저정보를 들고 있음

	// 검색된 방 정보를 Blueprint 로 공개할 수 있도록 하는 배열
	UPROPERTY(BlueprintReadOnly)
	TArray<FSessionData>				m_SessionData;


	bool					m_bSessionCreated;
	bool					m_bSessionSerching;
	bool					m_bSessionJoin;


	// 델리게이트 핸들
	FDelegateHandle			m_OnCreateSessionHandle;
	FDelegateHandle			m_OnFindSessionHandle;
	FDelegateHandle			m_OnJoinSessionHandle;

	// 블루프린트로 뿌려줄 Delegate 선언
	UPROPERTY(BlueprintAssignable)
	FOnSessionOperationComplete	OnCreateSession_Blueprint;

	UPROPERTY(BlueprintAssignable)
	FOnSessionOperationComplete	OnFindSession_Blueprint;

	UPROPERTY(BlueprintAssignable)
	FOnSessionOperationComplete	OnJoinSession_Blueprint;


public:
	// GameInstanceSubsystem 초기화 함수. 생성 시점에 한번 호출이 들어온다.
	virtual void Initialize(FSubsystemCollectionBase& _Collection) override;


public:
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateSession(int32 _MaxPlayer);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindSession();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinSession(int32 _Idx);


public:
	void OnCreateSessionCompleted(FName _SessionName, bool _bSuccess);
	void OnFindSessionCompleted(bool _bSuccess);
	void OnJoinSessionCompleted(FName _SessionName, EOnJoinSessionCompleteResult::Type _Result);
};
