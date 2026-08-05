// Fill out your copyright notice in the Description page of Project Settings.


#include "C_SessionMgr.h"

void UC_SessionMgr::Initialize(FSubsystemCollectionBase& _Collection)
{
	Super::Initialize(_Collection);

	// OSS 생성
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();

	if (nullptr != OSS)
	{
		m_SessionInterface = OSS->GetSessionInterface();
	}
}

void UC_SessionMgr::CreateSession(int32 _MaxPlayer)
{
	// 이미 생성 시도중임
	if (m_bSessionCreated)
		return;

	// 만약에 이전에 생성한 세션이 남이있으면, 제거한다.
	if (m_SessionInterface->GetNamedSession(NAME_GameSession))
	{
		m_SessionInterface->DestroySession(NAME_GameSession);
	}

	FOnlineSessionSettings setting = {};

	setting.bIsLANMatch = true;						// Lan 환경
	setting.NumPublicConnections = _MaxPlayer;		// 최대 접속가능 플레이어 수
	setting.bShouldAdvertise = true;				// 공개
	setting.bAllowJoinInProgress = true;			// 접속 가능

	// 방 생성 시도 체크
	m_bSessionCreated = true;


	// 이전에 등록한 델리게이트가 있으면 제거
	m_SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(m_OnCreateSessionHandle);

	// 델리게이트 등록
	m_OnCreateSessionHandle = m_SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this
			, &UC_SessionMgr::OnCreateSessionCompleted)
	);

	// 방 생성 요청
	bool bResult = m_SessionInterface->CreateSession(0, NAME_GameSession, setting);

	if (false == bResult)
	{
		m_bSessionCreated = false;
		UE_LOG(LogTemp, Error, TEXT("!! CreateSession Failed !!"));
	}
}

void UC_SessionMgr::OnCreateSessionCompleted(FName _SessionName, bool _bSuccess)
{
	m_bSessionCreated = false;

	OnCreateSession_Blueprint.Broadcast(_bSuccess);
}

void UC_SessionMgr::FindSession()
{
	if (m_bSessionSerching)
		return;

	m_bSessionSerching = true;

	// 결과를 받을 객체가 존재하지 않으면, 동적할당하고 스마트 포인터로 가리킨다.
	// FOnlineSessionSearch 는 UObject 에서 파생되지 않았기 때문에 언리얼 GC(가비지 컬랙터) 의 메모리 관리를 받을 수 없다.
	// 따라서 전통적인 스마트 포인터로 레퍼런스 카운트를 관리하는 구조로 간다.
	if (false == m_SesionSearchResult.IsValid())
	{
		m_SesionSearchResult = MakeShared<FOnlineSessionSearch>();
	}

	m_SesionSearchResult->bIsLanQuery = true;
	m_SesionSearchResult->MaxSearchResults = 10;

	// 델리게이트 직전 등록
	m_SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(m_OnFindSessionHandle);

	m_OnFindSessionHandle = m_SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this
			, &UC_SessionMgr::OnFindSessionCompleted)
	);

	// 이전에 검색한 정보 클리어
	m_SessionData.Empty();

	// 탐색 요청
	m_SessionInterface->FindSessions(0, m_SesionSearchResult.ToSharedRef());
}

void UC_SessionMgr::OnFindSessionCompleted(bool _bSuccess)
{
	m_bSessionSerching = false;

	if (false == m_SesionSearchResult.IsValid())
		return;

	// 방 탐색이 성공 했다면
	if (_bSuccess)
	{
		for (const FOnlineSessionSearchResult& Result : m_SesionSearchResult->SearchResults)
		{
			FSessionData Data = {};

			Data.HostName = Result.Session.OwningUserName;

			Data.MaxPlayerNum = Result.Session.SessionSettings.NumPublicConnections;

			int32 Open = Result.Session.NumOpenPublicConnections;

			Data.CurPlayerNum = Data.MaxPlayerNum - Open;

			Data.Ping = Result.PingInMs;

			m_SessionData.Add(Data);
		}
	}

	// 블루프린트 쪽으로 전파
	OnFindSession_Blueprint.Broadcast(_bSuccess);
}


void UC_SessionMgr::JoinSession(int32 _Idx)
{
	if (m_bSessionJoin || !m_SessionInterface.IsValid())
		return;

	// 인덱스가 방 개수를 초과하는 경우
	if (m_SessionData.Num() <= _Idx)
		return;

	// 선택한 방(세션) 포인터를 얻어온다.
	const FOnlineSessionSearchResult& SelectedSession = m_SesionSearchResult->SearchResults[_Idx];

	if (!SelectedSession.IsValid())
		return;

	// 직전에 델리게이트 등록
	m_SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(m_OnJoinSessionHandle);

	m_OnJoinSessionHandle = m_SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this
			, &UC_SessionMgr::OnJoinSessionCompleted)
	);

	m_bSessionJoin = true;

	// JoinSession 호출
	m_SessionInterface->JoinSession(0, NAME_GameSession, SelectedSession);
}

void UC_SessionMgr::OnJoinSessionCompleted(FName _SessionName, EOnJoinSessionCompleteResult::Type _Result)
{
	m_bSessionJoin = false;

	// 방장과 네트워크가 연결이 성공했다면
	if (_Result == EOnJoinSessionCompleteResult::Success)
	{
		// 방장과 동일한 상태의 레벨로 전환한다.
		FString ConnectString;

		if (m_SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectString))
		{
			APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();

			if (PC)
			{
				PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
			}
		}
	}

	OnJoinSession_Blueprint.Broadcast(_Result == EOnJoinSessionCompleteResult::Success);
}