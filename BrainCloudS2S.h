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

//#include "BrainCloudS2S.generated.h"

class IHttpRequest;

using US2SCallback = std::function<void(const FString&)>;

//UCLASS(MinimalAPI)
class UBrainCloudS2S //: public UObject
{
    //GENERATED_BODY()

public:
    //Base Constructor for those using NewObject or createSubobject
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
    void request(const FString &jsonString, const US2SCallback &callback);

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

    enum class State : uint8 {
        Disconnected = 0,
        Authenitcating = 1,
        Authenticated = 2
    };

private:
    struct Request
    {
        FString jsonString;
        US2SCallback callback;
        TSharedPtr<IHttpRequest> pHTTPRequest;
    };

    void queueRequest(const TSharedPtr<Request> &pRequest);
    void sendHeartbeat();

    void onAuthenticateCallback(const FString &jsonString);
    void onHeartbeatCallback(const FString &jsonString);

    void CheckAuthCredentials(TSharedPtr<FJsonObject> authResponse);

    FString _appId;
    FString _serverName;
    FString _serverSecret;
    FString _url;
    
    TSharedPtr<Request> _activeRequest;
    TSharedPtr<Request> _nullActiveRequest;
    State _state = State::Disconnected;
    bool _autoAuth = false;
    bool _logEnabled = false;
    int32 _packetId = 0;
    FString _sessionId = "";
    
    double _heartbeatStartTime = 0;
    double _heartbeatInverval;

    TArray<TSharedPtr<Request>> _requestQueue;
};

DECLARE_LOG_CATEGORY_EXTERN(LogBrainCloudS2S, Log, All);
