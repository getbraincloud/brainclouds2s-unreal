// Copyright 2023 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "S2SSocket.h"
#include "BrainCloudS2S.h"

#include "S2SRTTComms.generated.h"

#define INITIAL_HEARTBEAT_TIME 10

class BRAINCLOUDS2SPLUGIN_API IRTTCallback
{
public:
	/**
	 */
	virtual void rttCallback(const FString& jsonData) = 0;
};

class US2SCommsProxy;
class US2SSocket;


UCLASS(Blueprintable, BlueprintType)
class BRAINCLOUDS2SPLUGIN_API US2SRTTComms : public UObject
{
	GENERATED_BODY()
public:
	US2SRTTComms();
	~US2SRTTComms();

	/*
	 * InitializeS2S - used to initialize s2s app settings if needed.
	 * @param appId Application ID
	 * @param serverName Server name
	 * @param serverSecret Server secret key
	 * @param url The server url to send the request to.
	 */
	void InitializeS2S(const FString& appId,
		const FString& serverName,
		const FString& serverSecret,
		const FString& url,
		bool autoAuth,
		bool logEnabled);

	/*
	 * Update and perform callbacks on the calling thread.
	 */
	void runCallbacks();

	/*
	 *  Disconnects the RTT socket
	 */
	void disconnect();

	/*
	 * enableRTT - enables RTT by getting the connection information for the WebSocket connection and connects to it
	 * @param OnSuccess Callback for when RTT successfully enables
	 * @param OnFailure Callback for when RTT fails to enable
	 */
	void enableRTT(const US2SCallback& OnSuccess, const US2SCallback& OnFailure);

	/*
	 * Disables RTT
	 */
	void disableRTT();

	/*
	 * request - makes an S2S request 
	 * @param requestJson the json data string sent in the request
	 * @param Callback Callback for when we get a response
	 */
	void request(const FString& requestJson, const US2SCallback& Callback);

	/*
	 * authenticate - authenticates and starts the S2S session
	 * @param callback Callback for the authentication response
	 */
	void authenticate(const US2SCallback& callback);

	/*
	 * authenticate - authenticates and starts the S2S session
	 */
	void authenticate();

	/*
	 * registerRTTCallback - registers an RTT callback that is triggered when we receive an RTT message
	 * @param callback Callback for when an RTT message is received
	 */
	void registerRTTCallback(const US2SCallback& callback);

	/*
	 * deregisterRTTCallback - deregisters the set RTT callback
	 */
	void deregisterRTTCallback();

	/*
	 * send - sends an RTT message using the WebSocket connection
	 * @param in_message the message sent
	 * @param in_allowLogging whether or not to print the debug logs
	 */
	void send(const FString& in_message, bool in_allowLogging = true);

	/*
	 * joinSystemChatChannel - joins the system channel to receive and send system chat messages
	 * @param callback callback for the response received
	 */
	void joinSystemChatChannel(const US2SCallback& callback);

	/*
	 * webSocket_OnClose - callback function for when the web socket closes
	 */
	UFUNCTION()
	void webSocket_OnClose();
	/*
	 * websocket_OnOpen - callback function for when the web socket successfully connects
	 */
	UFUNCTION()
	void websocket_OnOpen();
	/*
	 * webSocket_OnMessage - callback function for when the web socket receives a packet
	 */
	UFUNCTION()
	void webSocket_OnMessage(const TArray<uint8>& in_data);
	/*
	 * webSocket_OnError - callback function for when there is an error with the web socket connection
	 */
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




