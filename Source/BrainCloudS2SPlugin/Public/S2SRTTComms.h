// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "S2SSocket.h"
#include "BrainCloudS2S.h"
#include "BrainCloudS2STypes.h"

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

	/** Called internally by UBrainCloudS2S to wire this service to its owning context. */
	void SetS2SContext(UBrainCloudS2S* context);

	/*
	 * Update RTT heartbeats and check for disconnection.
	 * Called by UBrainCloudS2S::runCallbacks().
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

	// -----------------------------------------------------------------------
	// Blueprint wrappers (S2S_ prefix)
	// -----------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|RTT", meta = (DisplayName = "Enable RTT"))
	void S2S_EnableRTT(const FS2SResponseDelegate& OnSuccess, const FS2SResponseDelegate& OnFailure);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|RTT", meta = (DisplayName = "Disable RTT"))
	void S2S_DisableRTT();

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|RTT", meta = (DisplayName = "Register RTT Callback"))
	void S2S_RegisterRTTCallback(const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|RTT", meta = (DisplayName = "Deregister RTT Callback"))
	void S2S_DeregisterRTTCallback();

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|RTT", meta = (DisplayName = "Send RTT Message"))
	void S2S_Send(const FString& Message, bool bAllowLogging = true);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|RTT", meta = (DisplayName = "Disconnect RTT"))
	void S2S_Disconnect();

private:

	FString getUrlQueryParameters();
	void setupWebSocket(const FString& in_url);
	void setRTTHeartBeatSeconds(int32 in_value);

	FString buildConnectionRequest();
	FString buildHeartbeatRequest();

	void processRTTCallback(const FString &in_message);

	/** Helper: wraps an FS2SResponseDelegate into a US2SCallback (success is always true for RTT messages). */
	static US2SCallback WrapBPDelegate(const FS2SResponseDelegate& Delegate);

	US2SSocket *s2sSocket = nullptr;

	FString m_cxId;
	FString m_eventServer;

	UPROPERTY()
	UBrainCloudS2S* m_s2sClient = nullptr;
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
