// Copyright 2023 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "S2SSocket.h"
#include "BrainCloudS2S.h"
#include "S2SRTTComms.generated.h"

#define INITIAL_HEARTBEAT_TIME 10

class IRTTCallback
{
public:
	/**
	 */
	virtual void rttCallback(const FString& jsonData) = 0;
};

class US2SCommsProxy;
class US2SSocket;


UCLASS(MinimalAPI)
class US2SRTTComms : public UObject
{
	GENERATED_BODY()

public:
	US2SRTTComms();
	~US2SRTTComms();

	void InitializeS2S(const FString& appId,
		const FString& serverName,
		const FString& serverSecret,
		const FString& url,
		bool autoAuth,
		bool logEnabled);

	void runCallbacks();

	void disconnect();

	void enableRTT(const US2SCallback& OnSuccess, const US2SCallback& OnFailure);

	void disableRTT();

	void request(const FString& requestJson, const US2SCallback& Callback);

	void authenticate(const US2SCallback& callback);

	void authenticate();

	void registerRTTCallback(const US2SCallback& callback);

	void deregisterRTTCallback();

	void send(const FString& in_message, bool in_allowLogging = true);

	void joinSystemChatChannel(const US2SCallback& callback);

	UFUNCTION()
	void webSocket_OnClose();
	UFUNCTION()
	void websocket_OnOpen();
	UFUNCTION()
	void webSocket_OnMessage(const TArray<uint8>& in_data);
	UFUNCTION()
	void webSocket_OnError(const FString& in_error);

private:

	FString getUrlQueryParameters();
	void setupWebSocket(const FString& in_url);
	void setRTTHeartBeatSeconds(int32 in_value);
	
	FString buildConnectionRequest();
	FString buildHeartbeatRequest();

	void processRTTCallback(const FString &in_message);

	US2SSocket *s2sSocket;

	FString m_cxId;
	FString m_eventServer;

	TSharedPtr<UBrainCloudS2S> m_s2sClient;
	TSharedPtr<FJsonObject> m_rttHeaders;
	TSharedRef<FJsonObject> m_disconnectJson = MakeShareable(new FJsonObject());

	float m_heartBeatSecs;
	float m_heartBeatIdleSecs;
	float m_timeSinceLastRequest;
	float m_lastNowMS;
	bool m_heartBeatRecv = true;
	bool m_heartBeatSent = false;

	bool m_bIsConnected;
	bool m_disconnectedWithReason = false;

	bool isRTTEnabled = false;

	US2SCallback registeredBPCallback;
	US2SCallback rttEnabledSuccessCallback;
	US2SCallback rttEnabledFailureCallback;
};




