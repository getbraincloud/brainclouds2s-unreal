/*
    2020 @ bitheads inc.
    Author: David St-Louis
*/

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

#include <functional>
#include <memory>
#include <string>

class IHttpRequest;

using US2SCallback = std::function<void(const FString&)>;

class UBrainCloudS2S
{
public:
    /*
     * Constructor
     * @param appId Application ID
     * @param serverName Server name
     * @param serverSecret Server secret key
     * @param url The server url to send the request to.
     */
    UBrainCloudS2S(const FString &appId,
                   const FString &serverName,
                   const FString &serverSecret,
                   const FString &url = "https://sharedprod.braincloudservers.com/s2sdispatcher");

    virtual ~UBrainCloudS2S();

    /*
     * Set wether S2S messages and errors are logged to the console
     * @param enabled Will log if true. Default false
     */
    void setLogEnabled(bool enabled);

    /*
     * Send an S2S request.
     * @param json Content to be sent
     * @param callback Callback function
     */
    void request(const FString &jsonString, const US2SCallback &callback);

    /*
     * Update and perform callbacks on the calling thread.
     */
    void runCallbacks();

private:
    struct Request
    {
        FString jsonString;
        US2SCallback callback;
        TSharedPtr<IHttpRequest> pHTTPRequest;
    };

    void disconnect();

    void sendRequest(const TSharedPtr<Request> &pRequest);
    void sendHeartbeat();

    void onAuthenticateCallback(const FString &jsonString);
    void onHeartbeatCallback(const FString &jsonString);

    FString _appId;
    FString _serverName;
    FString _serverSecret;
    FString _url;
    
    bool _logEnabled = false;
    bool _authenticated = false;
    int32 _packetId = 0;
    FString _sessionId = "";
    
    double _heartbeatStartTime = 0;
    double _heartbeatInverval;

    TArray<TSharedPtr<Request> > _requestQueue;
};

DECLARE_LOG_CATEGORY_EXTERN(LogBrainCloudS2S, Log, All);
