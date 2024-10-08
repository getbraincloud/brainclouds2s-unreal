#include "BrainCloudS2S.h"
#include "Logging/LogMacros.h"
#include "Http.h"
#include "Json.h"

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.
#include "Modules/ModuleManager.h"


// Error code for expired session
static const int SERVER_SESSION_EXPIRED = 40365;

// 30 minutes heartbeat interval
static const int HEARTBEAT_INTERVALE_S = 60 * 30;

UBrainCloudS2S::UBrainCloudS2S()
{
}

UBrainCloudS2S::UBrainCloudS2S(const FString& appId,
    const FString& serverName,
    const FString& serverSecret,
    const FString& url,
    bool autoAuth)
{
    _sessionData.appId = appId;
    _sessionData.serverName = serverName;
    _sessionData.serverSecret = serverSecret;
    _sessionData.url = url;
    _sessionData.state = S2SState::Disconnected;
    _autoAuth = autoAuth;
    _sessionData.heartbeatInterval = HEARTBEAT_INTERVALE_S;
}

UBrainCloudS2S::~UBrainCloudS2S()
{
    for (int32 i = 0; i < _requestQueue.Num(); ++i)
    {
        if (_requestQueue[i]->pHTTPRequest)
        {
            _requestQueue[i]->pHTTPRequest->CancelRequest();
        }
    }
    _requestQueue.Empty();
}

void UBrainCloudS2S::Init(const FString& appId,
    const FString& serverName,
    const FString& serverSecret,
    const FString& url,
    bool autoAuth)
{
    _sessionData.appId = appId;
    _sessionData.serverName = serverName;
    _sessionData.serverSecret = serverSecret;
    _sessionData.url = url;
    _autoAuth = autoAuth;
    _sessionData.state = S2SState::Disconnected;
    _sessionData.heartbeatInterval = HEARTBEAT_INTERVALE_S;
}

void UBrainCloudS2S::authenticate(const US2SCallback& callback)
{
    if (_sessionData.state != S2SState::Authenticated)
    {
        _sessionData.state = S2SState::Authenticating;
        FString jsonAuthString = "{\"service\":\"authenticationV2\",\"operation\":\"AUTHENTICATE\",\"data\":{\"appId\":\"" + _sessionData.appId + "\",\"serverName\":\"" + _sessionData.serverName + "\",\"serverSecret\":\"" + _sessionData.serverSecret + "\"}}";
        TSharedPtr<Request> pAuthRequest(new Request{
            jsonAuthString,
            callback,
            nullptr
            });
        _requestQueue.Add(pAuthRequest);
        queueRequest(pAuthRequest);
    }
}

void UBrainCloudS2S::authenticate()
{
    authenticate(std::bind(&UBrainCloudS2S::onAuthenticateCallback, this, std::placeholders::_1));
}

void UBrainCloudS2S::disconnect()
{
    _sessionData.state = S2SState::Disconnected;
    _sessionData.packetId = 0;
    _sessionData.sessionId = "";

}

FString UBrainCloudS2S::getSessionID()
{
    return _sessionData.sessionId;
}

US2SSessionData UBrainCloudS2S::getSessionData()
{
    return _sessionData;
}

void UBrainCloudS2S::setLogEnabled(bool enabled)
{
    _logEnabled = enabled;
}

void UBrainCloudS2S::sendHeartbeat()
{
    FString jsonHeartbeatString = "{\"service\":\"heartbeat\",\"operation\":\"HEARTBEAT\"}";
    TSharedPtr<Request> pAuthRequest(new Request{
        jsonHeartbeatString,
        std::bind(&UBrainCloudS2S::onHeartbeatCallback, this, std::placeholders::_1),
        nullptr
        });
    _requestQueue.Add(pAuthRequest);
    queueRequest(pAuthRequest);
}

void UBrainCloudS2S::queueRequest(const TSharedPtr<Request>& pRequest)
{
    FString dataString = "{\"packetId\":" + FString::FromInt(_sessionData.packetId);
    if (_sessionData.sessionId.Len())
    {
        dataString += ",\"sessionId\":\"" + _sessionData.sessionId + "\"";
    }
    dataString += ",\"messages\":[" + pRequest->jsonString + "]}";

    if (_logEnabled)
    {
        UE_LOG(LogBrainCloudS2S, Log, TEXT("Sending request:%s\n"), *dataString);
    }

    pRequest->pHTTPRequest = FHttpModule::Get().CreateRequest();
    pRequest->pHTTPRequest->SetURL(_sessionData.url);
    pRequest->pHTTPRequest->SetVerb(TEXT("POST"));
    pRequest->pHTTPRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    pRequest->pHTTPRequest->SetContentAsString(dataString);
    pRequest->pHTTPRequest->ProcessRequest();

    ++_sessionData.packetId;
}

void UBrainCloudS2S::request(const FString& jsonString, const US2SCallback& callback)
{
    // If autoAuth is on 
    UE_LOG(LogBrainCloudS2S, Log, TEXT("[S2SRequest] autoAuth: %s \n"), _autoAuth ? TEXT("true") : TEXT("false"));
    if (_autoAuth)
    {
        if (_sessionData.state != S2SState::Authenticated && !_requestQueue.Num())
        {
            authenticate(std::bind(&UBrainCloudS2S::onAuthenticateCallback, this, std::placeholders::_1));
        }
    }

    // Queue the request
    TSharedPtr<Request> pRequest(new Request{ jsonString, callback, nullptr });
    _requestQueue.Add(pRequest);

    // If we are the only thing in queue, send it now
    if (_requestQueue.Num() == 1 && _sessionData.state == S2SState::Authenticated)
    {
        queueRequest(pRequest);
    }
}

void UBrainCloudS2S::onHeartbeatCallback(const FString& jsonString)
{
    // Try to deserialize the json
    TSharedRef<TJsonReader<TCHAR>> reader = TJsonReaderFactory<TCHAR>::Create(jsonString);
    TSharedPtr<FJsonObject> pMessage = MakeShareable(new FJsonObject());
    if (FJsonSerializer::Deserialize(reader, pMessage))
    {
        if (pMessage->HasField("status"))
        {
            if (pMessage->GetIntegerField("status") != 200)
            {
                return; // All good
            }
        }
    }

    disconnect(); // Something went wrong
}

void UBrainCloudS2S::CheckAuthCredentials(TSharedPtr<FJsonObject> authResponse)
{
    if (authResponse && authResponse->HasField("data"))
    {
        const auto& pData = authResponse->GetObjectField("data");
        if (pData->HasField("heartbeatSeconds"))
        {
            _sessionData.heartbeatInterval = pData->GetNumberField("heartbeatSeconds");
            //_heartbeatInverval = 300;
        }
        if (pData->HasField("sessionId"))
        {
            _sessionData.sessionId = pData->GetStringField("sessionId");
        }
        _sessionData.heartbeatStartTime = FPlatformTime::Seconds();
        _sessionData.state = S2SState::Authenticated;

        if (_logEnabled)
            UE_LOG(LogBrainCloudS2S, Log, TEXT("S2S Authenticated - set heartbeatInterval to: %.0lf"), _sessionData.heartbeatInterval);
    }
    else
    {
        if (_logEnabled)
            UE_LOG(LogBrainCloudS2S, Error, TEXT("S2S Failed To Authenticate"));
    }
}

void UBrainCloudS2S::onAuthenticateCallback(const FString& jsonString)
{
    if(_logEnabled)
        UE_LOG(LogBrainCloudS2S, Log, TEXT("S2S Authenticate result: %s"), *jsonString);

    if (_sessionData.state != S2SState::Authenticated)
    {
        // Try to deserialize the json
        TSharedRef<TJsonReader<TCHAR>> reader = TJsonReaderFactory<TCHAR>::Create(jsonString);
        TSharedPtr<FJsonObject> pMessage = MakeShareable(new FJsonObject());
        if (FJsonSerializer::Deserialize(reader, pMessage))
        {
            CheckAuthCredentials(pMessage);
        }
        else
        {
            // msg is from braincloud, this should never happen
            UE_LOG(LogBrainCloudS2S, Error, TEXT("S2S Invalid JSON: %s"), *jsonString);
        }
    }
}

void UBrainCloudS2S::runCallbacks()
{
    if (_activeRequest == nullptr)
    {
        if (_requestQueue.Num())
        {
            _activeRequest = _requestQueue[0];
        }
    }
    else
    {
        if(_logEnabled)
            UE_LOG(LogBrainCloudS2S, Log, TEXT("Number of Waiting Requests, %d"), _requestQueue.Num()); //Check num request in queue

        //auto pActiveRequest = _requestQueue[0];
        auto pActiveRequest = _activeRequest;
        EHttpRequestStatus::Type status = pActiveRequest->pHTTPRequest->GetStatus();

        if (status == EHttpRequestStatus::Succeeded)
        {
            FHttpResponsePtr resp = pActiveRequest->pHTTPRequest->GetResponse();
            if (resp.IsValid())
            {
                auto responseCode = resp->GetResponseCode();
                const auto& responseBody = resp->GetContentAsString();
                FString responseMessage;

                // Get the inner body of the message
                TSharedRef<TJsonReader<TCHAR>> reader = TJsonReaderFactory<TCHAR>::Create(responseBody);
                TSharedPtr<FJsonObject> jsonPacket = MakeShareable(new FJsonObject());
                TSharedPtr<FJsonObject> jsonMessage;
                if (FJsonSerializer::Deserialize(reader, jsonPacket))
                {
                    if (jsonPacket->HasField("messageResponses"))
                    {
                        const auto& messages = jsonPacket->GetArrayField("messageResponses");
                        if (messages.Num())
                        {
                            jsonMessage = messages[0]->AsObject();
                            TSharedRef<TJsonWriter<> > writer = TJsonWriterFactory<>::Create(&responseMessage);
                            FJsonSerializer::Serialize(jsonMessage.ToSharedRef(), writer);

                            if (_sessionData.state != S2SState::Authenticated) // will only do this on an auth call
                            {
                                CheckAuthCredentials(jsonMessage);
                            }
                        }
                    }
                }
                else
                {
                    // msg is from braincloud, this should never happen
                    UE_LOG(LogBrainCloudS2S, Error, TEXT("S2S Invalid JSON: %s"), *responseBody);
                }

                if (responseCode == 200)
                {
                    if (_logEnabled)
                    {
                        UE_LOG(LogBrainCloudS2S, Log, TEXT("S2S Response: %s"), *responseMessage);
                    }

                    // Callback
                    if (pActiveRequest->callback)
                    {
                        pActiveRequest->callback(responseMessage);
                    }

                    // Remove and process next
                    _requestQueue.RemoveAt(0);
                    _activeRequest = nullptr;//dispose of the current active request

                    if (_requestQueue.Num())
                    {
                        queueRequest(_requestQueue[0]);
                    }
                }
                else
                {
                    // If it's a session expired, we disconnect
                    if (jsonMessage && jsonMessage->HasField("reason_code"))
                    {
                        if (jsonMessage->GetIntegerField("reason_code") == SERVER_SESSION_EXPIRED)
                        {
                            // Disconect then redo the request. It will try to re-authenticate
                            UE_LOG(LogBrainCloudS2S, Warning, TEXT("S2S session expired"));
                            pActiveRequest->pHTTPRequest->CancelRequest();
                            pActiveRequest->pHTTPRequest.Reset();
                            disconnect();
                            if (!_autoAuth) //need to re-auth here for them if we want to re-request and not dequeue
                            {
                                authenticate(std::bind(&UBrainCloudS2S::onAuthenticateCallback, this, std::placeholders::_1));
                            }
                            queueRequest(pActiveRequest); // Re-request, dont dequeue, if auto auth is true, it will not need to have an additional auth
                            return;
                        }
                    }

                    UE_LOG(LogBrainCloudS2S, Error, TEXT("S2S Failed: %s"), *responseBody);
                    pActiveRequest->pHTTPRequest->CancelRequest();

                    // Callback
                    if (pActiveRequest->callback)
                    {
                        pActiveRequest->callback(responseMessage);
                    }

                    // Remove and process next
                    _requestQueue.RemoveAt(0);
                    _activeRequest = nullptr; //dispose of the current active request

                    if (_requestQueue.Num())
                    {
                        queueRequest(_requestQueue[0]);
                    }
                }
            }
        }
        else if (status == EHttpRequestStatus::Failed)
        {
            UE_LOG(LogBrainCloudS2S, Error, TEXT("S2S Failed"));
            pActiveRequest->pHTTPRequest->CancelRequest();

            // Callback
            if (pActiveRequest->callback)
            {
                pActiveRequest->callback("{\"status_code\":900,\"message\":\"HTTP Request failed\"}");
            }

            // Remove and process next
            _requestQueue.RemoveAt(0);
            _activeRequest = nullptr; //dispose of the current active request

            if (_requestQueue.Num())
            {
                queueRequest(_requestQueue[0]);
            }
        }
        else if (status == EHttpRequestStatus::Processing)
        {
            // Check for timeout
        }
    }

    //Send heartbeat if we have to
    if (_sessionData.state == S2SState::Authenticated)
    {
        auto now = FPlatformTime::Seconds();
        auto timeDiff = now - _sessionData.heartbeatStartTime;

        FString timeDiffstr = FString::SanitizeFloat(timeDiff);
        FString heartbeatIntervalStr = FString::SanitizeFloat(_sessionData.heartbeatInterval);
        FString heartbeatStartTimeStr = FString::SanitizeFloat(_sessionData.heartbeatStartTime);

        if (timeDiff >= _sessionData.heartbeatInterval)
        {
            sendHeartbeat();
            _sessionData.heartbeatStartTime = now;
        }
    }
}
