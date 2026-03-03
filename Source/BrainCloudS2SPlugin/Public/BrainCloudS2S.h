// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "BrainCloudS2STypes.h"

#include <functional>
#include <memory>
#include <string>

#include "BrainCloudS2S.generated.h"

/**
 *
 */

class IHttpRequest;
class US2SGlobalFileV3;
class US2SRTTComms;

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


UCLASS(BlueprintType)
class BRAINCLOUDS2SPLUGIN_API UBrainCloudS2S : public UObject
{
    GENERATED_BODY()

public:
	UBrainCloudS2S();

	virtual ~UBrainCloudS2S();

    // -----------------------------------------------------------------------
    // Factory
    // -----------------------------------------------------------------------

    /**
     * Creates and initializes a new S2S context. This is the recommended way
     * to create an instance from both C++ and Blueprints.
     *
     * @param AppId          Application ID
     * @param ServerName     Server name
     * @param ServerSecret   Server secret key
     * @param Url            The server url to send the request to.
     * @param bAutoAuth      When true, authentication happens automatically on the first request.
     * @return A fully initialized UBrainCloudS2S instance.
     */
    UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S", meta = (DisplayName = "Create S2S Context"))
    static UBrainCloudS2S* CreateS2SContext(
        const FString& AppId,
        const FString& ServerName,
        const FString& ServerSecret,
        const FString& Url,
        bool bAutoAuth);

    // -----------------------------------------------------------------------
    // Original C++ API (signatures preserved)
    // -----------------------------------------------------------------------

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

    // -----------------------------------------------------------------------
    // Service accessors
    // -----------------------------------------------------------------------

    /** Returns the Global File V3 service owned by this context. */
    UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S", meta = (DisplayName = "Get Global File V3"))
    US2SGlobalFileV3* GetGlobalFileV3() const;

    /** Returns the RTT communications service owned by this context. */
    UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S", meta = (DisplayName = "Get RTT Comms"))
    US2SRTTComms* GetRTTComms() const;

    // -----------------------------------------------------------------------
    // Blueprint-callable wrappers
    // -----------------------------------------------------------------------

    /** Blueprint-callable version of runCallbacks(). Drives all service callbacks and HTTP completions. */
    UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S", meta = (DisplayName = "Run Callbacks"))
    void RunCallbacks();

    UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S", meta = (DisplayName = "Set Log Enabled"))
    void S2S_SetLogEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S", meta = (DisplayName = "Request"))
    void S2S_Request(const FString& JsonString, const FS2SResponseDelegate& Callback);

    UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S", meta = (DisplayName = "Authenticate"))
    void S2S_Authenticate(const FS2SResponseDelegate& Callback);

    UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S", meta = (DisplayName = "Authenticate Auto"))
    void S2S_AuthenticateAuto();

    UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S", meta = (DisplayName = "Disconnect"))
    void S2S_Disconnect();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BrainCloud|S2S", meta = (DisplayName = "Get Session ID"))
    FString S2S_GetSessionID() const;

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

    /** Helper: wraps an FS2SResponseDelegate into a US2SCallback. */
    static US2SCallback WrapBPDelegate(const FS2SResponseDelegate& Delegate);

    /** Creates and initializes owned service sub-objects. Called after Init(). */
    void InitServices();

    TSharedPtr<Request> _activeRequest;
    TSharedPtr<Request> _nullActiveRequest;
    bool _autoAuth = false;
    bool _logEnabled = false;

    US2SSessionData _sessionData;

    TArray<TSharedPtr<Request>> _requestQueue;

    // Owned service instances
    UPROPERTY()
    US2SGlobalFileV3* _globalFileV3 = nullptr;

    UPROPERTY()
    US2SRTTComms* _rttComms = nullptr;
};
