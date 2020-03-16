#include "BrainCloudS2S.h"

#include "Logging/LogMacros.h"
#include "Http.h"
#include "Json.h"

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogBrainCloudS2S);

// Error code for expired session
static const int SERVER_SESSION_EXPIRED = 40365;

// 30 minutes heartbeat interval
static const int HEARTBEAT_INTERVALE_S = 60.0 * 30.0;

UBrainCloudS2S::UBrainCloudS2S(const FString& appId,
                               const FString& serverName,
                               const FString& serverSecret,
                               const FString& url)
    : _appId(appId)
    , _serverName(serverName)
    , _serverSecret(serverSecret)
    , _url(url)
    , _heartbeatInverval(HEARTBEAT_INTERVALE_S)
{
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

void UBrainCloudS2S::disconnect()
{
    _authenticated = false;
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
    sendRequest(pAuthRequest);
}

void UBrainCloudS2S::sendRequest(const TSharedPtr<Request> &pRequest)
{
    FString dataString = "{\"packetId\":" + FString::FromInt(_packetId);
    if (_sessionId.Len())
    {
        dataString += ",\"sessionId\":\"" + _sessionId + "\"";
    }
    dataString += ",\"messages\":[" + pRequest->jsonString + "]}";

    if (_logEnabled)
    {
        UE_LOG(LogBrainCloudS2S, Log, TEXT("Sending request:%s\n"), *dataString);
    }

    pRequest->pHTTPRequest = FHttpModule::Get().CreateRequest();
    pRequest->pHTTPRequest->SetURL(_url);
    pRequest->pHTTPRequest->SetVerb(TEXT("POST"));
    pRequest->pHTTPRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    pRequest->pHTTPRequest->SetContentAsString(dataString);
    pRequest->pHTTPRequest->ProcessRequest();

    ++_packetId;
}

void UBrainCloudS2S::request(const FString& jsonString, const US2SCallback& callback)
{
    // If not authenticated, do that first.
    // Also make sure hnothing else is queue, if it's queued then auth is probably in flight.
    if (!_authenticated && !_requestQueue.Num())
    {
        FString jsonAuthString = "{\"service\":\"authenticationV2\",\"operation\":\"AUTHENTICATE\",\"data\":{\"appId\":\"" + _appId + "\",\"serverName\":\"" + _serverName + "\",\"serverSecret\":\"" + _serverSecret + "\"}}";
        TSharedPtr<Request> pAuthRequest(new Request{ 
            jsonAuthString,
            std::bind(&UBrainCloudS2S::onAuthenticateCallback, this, std::placeholders::_1), 
            nullptr 
        });
        _requestQueue.Add(pAuthRequest);
        sendRequest(pAuthRequest);
    }

    // Queue the request
    TSharedPtr<Request> pRequest(new Request{ jsonString, callback, nullptr });
    _requestQueue.Add(pRequest);

    // If we are the only thing in queue, send it now
    if (_requestQueue.Num() == 1)
    {
        sendRequest(pRequest);
    }
}

void UBrainCloudS2S::onHeartbeatCallback(const FString &jsonString)
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

void UBrainCloudS2S::onAuthenticateCallback(const FString &jsonString)
{
    // Try to deserialize the json
    TSharedRef<TJsonReader<TCHAR>> reader = TJsonReaderFactory<TCHAR>::Create(jsonString);
    TSharedPtr<FJsonObject> pMessage = MakeShareable(new FJsonObject());
    if (FJsonSerializer::Deserialize(reader, pMessage))
    {
        if (pMessage->HasField("data"))
        {
            const auto &pData = pMessage->GetObjectField("data");
            if (pData->HasField("heartbeatSeconds"))
            {
                _heartbeatInverval = pData->GetNumberField("heartbeatSeconds");
            }
            if (pData->HasField("sessionId"))
            {
                _sessionId = pData->GetStringField("sessionId");
            }
            _heartbeatStartTime = FPlatformTime::Seconds();
            _authenticated = true;
            UE_LOG(LogBrainCloudS2S, Log, TEXT("S2S Authenticated"));
        }
    }
    else
    {
        // msg is from braincloud, this should never happen
        UE_LOG(LogBrainCloudS2S, Error, TEXT("S2S Invalid JSON: %s"), *jsonString);
    }
}

void UBrainCloudS2S::runCallbacks()
{
    if (_requestQueue.Num())
    {
        auto pActiveRequest = _requestQueue[0];
        EHttpRequestStatus::Type status = pActiveRequest->pHTTPRequest->GetStatus();

        if (status == EHttpRequestStatus::Succeeded)
        {
            FHttpResponsePtr resp = pActiveRequest->pHTTPRequest->GetResponse();
            if (resp.IsValid())
            {
                auto responseCode = resp->GetResponseCode();
                const auto &responseBody = resp->GetContentAsString();
                FString responseMessage;

                // Get the inner body of the message
                TSharedRef<TJsonReader<TCHAR>> reader = TJsonReaderFactory<TCHAR>::Create(responseBody);
                TSharedPtr<FJsonObject> jsonPacket = MakeShareable(new FJsonObject());
                TSharedPtr<FJsonObject> jsonMessage;
                if (FJsonSerializer::Deserialize(reader, jsonPacket))
                {
                    if (jsonPacket->HasField("messageResponses"))
                    {
                        const auto &messages = jsonPacket->GetArrayField("messageResponses");
                        if (messages.Num())
                        {
                            jsonMessage = messages[0]->AsObject();
                            TSharedRef<TJsonWriter<> > writer = TJsonWriterFactory<>::Create(&responseMessage);
                            FJsonSerializer::Serialize(jsonMessage.ToSharedRef(), writer);
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
                    if (_requestQueue.Num())
                    {
                        sendRequest(_requestQueue[0]);
                    }
                }
                else
                {
                    // If it's a session expired, we disconnect
                    if (jsonMessage->HasField("reason_code"))
                    {
                        if (jsonMessage->GetIntegerField("reason_code") == SERVER_SESSION_EXPIRED)
                        {
                            // Disconect then redo the request. It will try to re-authenticate
                            UE_LOG(LogBrainCloudS2S, Warning, TEXT("S2S session expired"));
                            pActiveRequest->pHTTPRequest->CancelRequest();
                            pActiveRequest->pHTTPRequest.Reset();
                            disconnect();
                            sendRequest(pActiveRequest); // Re-request, dont dequeue
                            return;
                        }
                    }

                    UE_LOG(LogBrainCloudS2S, Error, TEXT("S2S Failed: %s"), *responseMessage);
                    pActiveRequest->pHTTPRequest->CancelRequest();

                    // Callback
                    if (pActiveRequest->callback)
                    {
                        pActiveRequest->callback(responseMessage);
                    }

                    // Remove and process next
                    _requestQueue.RemoveAt(0);
                    if (_requestQueue.Num())
                    {
                        sendRequest(_requestQueue[0]);
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
                pActiveRequest->callback("{\"status_code\":900,\"message\":\"FTTP Request failed\"}");
            }

            // Remove and process next
            _requestQueue.RemoveAt(0);
            if (_requestQueue.Num())
            {
                sendRequest(_requestQueue[0]);
            }
        }
        else if (status == EHttpRequestStatus::Processing)
        {
            // Check for timeout
        }
    }

    // Send heartbeat if we have to
    if (_authenticated)
    {
        auto now = FPlatformTime::Seconds();
        auto timeDiff = now - _heartbeatStartTime;
        if (timeDiff >= _heartbeatInverval)
        {
            sendHeartbeat();
            _heartbeatStartTime = now;
        }
    }
}
