// Copyright 2023 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

#include <functional>
#include <memory>
#include <string>

/**
 * 
 */

class IHttpRequest;

using US2SCallback = std::function<void(const FString&)>;

enum S2SState : uint8 {
    Disconnected = 0,
    Authenticating = 1,
    Authenticated = 2
};

struct US2SSessionData
{
    FString appId;
    FString serverName;
    FString serverSecret;
    FString url;
    FString sessionId;
    int32 packetId = 0;
    double heartbeatStartTime = 0;
    double heartbeatInterval;
    S2SState state = S2SState::Disconnected;
};



class UBrainCloudS2S
{
public:
	UBrainCloudS2S();

    //Alternate constructor for those using MakeShareable
    UBrainCloudS2S(const FString& appId,
        const FString& serverName,
        const FString& serverSecret,
        const FString& url,
        bool autoAuth
    );

	virtual ~UBrainCloudS2S();

    /*
     * INIT - used to initialize s2s app settings if needed.
     * @param appId Application ID
     * @param serverName Server name
     * @param serverSecret Server secret key
     * @param url The server url to send the request to.
     */
    void Init(
        const FString& appId,
        const FString& serverName,
        const FString& serverSecret,
        const FString& url,
        bool autoAuth
    );

    /*
     * Set whether S2S messages and errors are logged to the console
     * @param enabled Will log if true. Default false
     */
    void setLogEnabled(bool enabled);

    /*
     * Send an S2S request.
     * @param json Content to be sent
     * @param callback Callback function
     */
    void request(const FString& jsonString, const US2SCallback& callback);

    /*
     * Update and perform callbacks on the calling thread.
     */
    void runCallbacks();

    /*
    * Authenticate with brainCloud. If autoAuth is set to false, which is
    * the default, this must be called successfully before doing other
    * requests. See S2SContext::create
    * @param callback Callback function
    */
    void authenticate(const US2SCallback& callback);

    /*
    Authenticate without custom callback, it will default to using
    ours instead.
    */
    void authenticate();

    /*
    * Disconnect
    */
    void disconnect();

    FString getSessionID();

    US2SSessionData getSessionData();

private:
    struct Request
    {
        FString jsonString;
        US2SCallback callback;
        TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> pHTTPRequest;
    };

    void queueRequest(const TSharedPtr<Request>& pRequest);
    void sendHeartbeat();

    void onAuthenticateCallback(const FString& jsonString);
    void onHeartbeatCallback(const FString& jsonString);

    void CheckAuthCredentials(TSharedPtr<FJsonObject> authResponse);

    TSharedPtr<Request> _activeRequest;
    TSharedPtr<Request> _nullActiveRequest;
    bool _autoAuth = false;
    bool _logEnabled = false;

    US2SSessionData _sessionData;

    TArray<TSharedPtr<Request>> _requestQueue;
};

