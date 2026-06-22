// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#include "S2SRTTComms.h"
#include "S2SServiceName.h"
#include "S2SServiceOperation.h"
#include "S2SOperationParam.h"
#include <Kismet/GameplayStatics.h>
#include "ConvertUtilities.h"
#include "JsonUtil.h"

#define MAX_PAYLOAD_RTT (64 * 1024) // [dsl] This used to be set to 10MB, failed on mac SNDBUF too big for the TCP socket.
#define INITIAL_HEARTBEAT_TIME 10
#define HEARTBEAT_IDLE_DELAY 2

US2SRTTComms::US2SRTTComms()
    : m_heartBeatSecs(INITIAL_HEARTBEAT_TIME)
    , m_heartBeatIdleSecs(HEARTBEAT_IDLE_DELAY)
    , m_timeSinceLastRequest(0)
    , m_lastNowMS(FPlatformTime::Seconds())
{
}

US2SRTTComms::~US2SRTTComms()
{
}

void US2SRTTComms::SetS2SContext(UBrainCloudS2S* context)
{
    m_s2sClient = context;
}

void US2SRTTComms::runCallbacks()
{
    // NOTE: S2S HTTP callbacks are driven by UBrainCloudS2S::runCallbacks(),
    // which calls us. We only handle RTT heartbeat / disconnect logic here.
    if (m_s2sClient == nullptr) return;

    if (!isRTTEnabled) return;

    if (s2sSocket != nullptr) {
        float nowMS = FPlatformTime::Seconds();

        m_timeSinceLastRequest += (nowMS - m_lastNowMS);
        m_lastNowMS = nowMS;

        if (m_heartBeatSent && m_timeSinceLastRequest >= m_heartBeatIdleSecs)
        {
            if (!m_heartBeatRecv) {
                UE_LOG(LogBrainCloudS2S, Log, TEXT("RTT: lost heartbeat %f idle"), m_heartBeatIdleSecs);
                disconnect();
            }
            m_heartBeatSent = false;
        }
        if (m_timeSinceLastRequest >= m_heartBeatSecs)
        {
            m_timeSinceLastRequest = 0;
            m_heartBeatSent = true;
            m_heartBeatRecv = false;
            send(buildHeartbeatRequest(), false);
        }
    }

    if (m_disconnectedWithReason)
    {
        disconnect();
    }
}

void US2SRTTComms::enableRTT(const US2SCallback& OnSuccess, const US2SCallback& OnFailure)
{
    if (m_s2sClient != nullptr) {

        TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

        JsonObject->SetStringField(TEXT("service"),   S2SServiceName::RTTRegistration);
        JsonObject->SetStringField(TEXT("operation"), S2SServiceOperation::RequestSystemConnection);

        FString JsonString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
        FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

        m_s2sClient->request(JsonString,
            [this, OnSuccess, OnFailure](const FString& result)
            {
                UE_LOG(LogBrainCloudS2S, Log, TEXT("Got response for connection request: %s"), *result);


                TSharedRef<TJsonReader<TCHAR>> reader = TJsonReaderFactory<TCHAR>::Create(result);
                TSharedPtr<FJsonObject> jsonPacket = MakeShareable(new FJsonObject());

                bool res = FJsonSerializer::Deserialize(reader, jsonPacket);
                int status = jsonPacket->GetIntegerField(TEXT("status"));
                if (res)
                {
                    TSharedPtr<FJsonObject> jsonData = jsonPacket->GetObjectField(TEXT("data"));
                    TArray<TSharedPtr<FJsonValue>> endpoints = jsonData->GetArrayField(S2SOperationParam::Endpoints);
                    m_rttHeaders = jsonData->GetObjectField(S2SOperationParam::Auth);

                    //setup socket connection
                    TSharedPtr<FJsonObject> endpoint = endpoints[0]->AsObject();
                    bool sslEnabled = endpoint->GetBoolField(S2SOperationParam::Ssl);

                    FString url = (sslEnabled ? TEXT("wss://") : TEXT("ws://"));
                    url += endpoint->GetStringField(S2SOperationParam::Host);
                    url += TEXT(":");
                    url += FString::Printf(TEXT("%d"), endpoint->GetIntegerField(S2SOperationParam::Port));
                    url += getUrlQueryParameters();

                    UE_LOG(S2SWebSocket, Log, TEXT("Setting up web socket with url %s "), *url);

                    setupWebSocket(url);
                    rttEnabledSuccessCallback = OnSuccess;
                    rttEnabledFailureCallback = OnFailure;

                }
                else if(status != 200){
                    OnFailure(result);
                }
            });
    }
}

void US2SRTTComms::disableRTT()
{
    if (s2sSocket != nullptr && s2sSocket->IsConnected()) {
        disconnect();
    }
    deregisterRTTCallback();
}

FString US2SRTTComms::getUrlQueryParameters()
{
    FString toReturn = TEXT("/?");
    int32 count = 0;

    // Iterate over Json Values
    for (auto currJsonValue = m_rttHeaders->Values.CreateConstIterator(); currJsonValue; ++currJsonValue)
    {
        // construct params
        if (count > 0)
            toReturn += TEXT("&");

        toReturn += (*currJsonValue).Key;
        toReturn += TEXT("=");
        TSharedPtr<FJsonValue> Value = (*currJsonValue).Value;
        toReturn += Value->AsString();

        ++count;
    }

    return toReturn;
}

void US2SRTTComms::setupWebSocket(const FString& in_url)
{
    if (s2sSocket == nullptr)
    {
        s2sSocket = NewObject<US2SSocket>();
        s2sSocket->AddToRoot();
    }

    m_timeSinceLastRequest = 0;

    s2sSocket->OnConnectError.AddDynamic(this, &US2SRTTComms::webSocket_OnError);
    s2sSocket->OnClosed.AddDynamic(this, &US2SRTTComms::webSocket_OnClose);
    s2sSocket->OnConnectComplete.AddDynamic(this, &US2SRTTComms::websocket_OnOpen);
    s2sSocket->OnReceiveData.AddDynamic(this, &US2SRTTComms::webSocket_OnMessage);

    s2sSocket->SetupSocket(in_url);
    s2sSocket->Connect();
}

void US2SRTTComms::setRTTHeartBeatSeconds(int32 in_value)
{
    m_heartBeatSecs = in_value;
    if (m_heartBeatIdleSecs > m_heartBeatSecs) m_heartBeatIdleSecs = m_heartBeatSecs;
}

void US2SRTTComms::send(const FString& in_message, bool in_allowLogging)
{
    // early return
    if (s2sSocket == nullptr || !s2sSocket->IsConnected())
    {
        return;
    }

    s2sSocket->SendText(in_message);
    if (in_allowLogging)
        UE_LOG(S2SWebSocket, Log, TEXT("RTT SEND:  %s"), *in_message);
}

void US2SRTTComms::joinSystemChatChannel(const US2SCallback& callback)
{

}

void US2SRTTComms::disconnect()
{
    if (m_s2sClient == nullptr) return;
    if (s2sSocket == nullptr) return;
    if (!s2sSocket->IsConnected()) return;

    // clear everything
    if (s2sSocket != nullptr)
    {
        s2sSocket->Close();
        s2sSocket->RemoveFromRoot();
        s2sSocket->OnConnectError.RemoveDynamic(this, &US2SRTTComms::webSocket_OnError);
        s2sSocket->OnClosed.RemoveDynamic(this, &US2SRTTComms::webSocket_OnClose);
        s2sSocket->OnConnectComplete.RemoveDynamic(this, &US2SRTTComms::websocket_OnOpen);
        s2sSocket->OnReceiveData.RemoveDynamic(this, &US2SRTTComms::webSocket_OnMessage);

    }

    if (s2sSocket)
        s2sSocket->ConditionalBeginDestroy();
    s2sSocket = nullptr;

    m_cxId = TEXT("");
    m_eventServer = TEXT("");
    m_disconnectedWithReason = false;
    m_heartBeatSent = false;
    m_heartBeatRecv = true;
    m_timeSinceLastRequest = 0;
    isRTTEnabled = false;
}

void US2SRTTComms::webSocket_OnMessage(const TArray<uint8>& in_data)
{
    FString parsedMessage = ConvertUtilities::BCBytesToString(in_data.GetData(), in_data.Num());
    UE_LOG(S2SWebSocket, Log, TEXT("RECV: %s "), *parsedMessage);

    TSharedPtr<FJsonObject> jsonData = JsonUtil::jsonStringToValue(parsedMessage);
    FString service   = jsonData->HasField(TEXT("service"))   ? jsonData->GetStringField(TEXT("service"))   : TEXT("");
    FString operation = jsonData->HasField(TEXT("operation")) ? jsonData->GetStringField(TEXT("operation")) : TEXT("");
    TSharedPtr<FJsonObject> innerData = nullptr;
    bool bIsInnerDataValid = jsonData->HasTypedField<EJson::Object>(TEXT("data"));

    if (bIsInnerDataValid)
        innerData = jsonData->GetObjectField(TEXT("data"));

    if (operation == S2SServiceOperation::Heartbeat)
    {
        m_heartBeatRecv = true;
    }

    if (operation == S2SServiceOperation::Connect)
    {
        int32 heartBeat = INITIAL_HEARTBEAT_TIME;
        if (bIsInnerDataValid && innerData->HasField(S2SOperationParam::HeartbeatSeconds))
        {
            heartBeat = innerData->GetIntegerField(S2SOperationParam::HeartbeatSeconds);
        }
        else if (bIsInnerDataValid && innerData->HasField(S2SOperationParam::WsHeartbeatSecs))
        {
            heartBeat = innerData->GetIntegerField(S2SOperationParam::WsHeartbeatSecs);
        }

        isRTTEnabled = true;
        setRTTHeartBeatSeconds(heartBeat);

        rttEnabledSuccessCallback(parsedMessage);
    }
    else if (operation == S2SServiceOperation::Disconnect)
    {
        m_disconnectedWithReason = true;
        m_disconnectJson->SetStringField(S2SOperationParam::Reason,     innerData->GetStringField(S2SOperationParam::Reason));
        m_disconnectJson->SetNumberField(TEXT("reasonCode"),            innerData->GetNumberField(TEXT("reasonCode")));
        m_disconnectJson->SetStringField(S2SOperationParam::Severity,   TEXT("ERROR"));
    }

    processRTTCallback(parsedMessage);

    if (bIsInnerDataValid)
    {
        if (innerData->HasField(S2SOperationParam::CxId))
        {
            m_cxId = innerData->GetStringField(S2SOperationParam::CxId);
        }

        if (innerData->HasField(S2SOperationParam::Evs))
        {
            m_eventServer = innerData->GetStringField(S2SOperationParam::Evs);
        }
    }

}

void US2SRTTComms::webSocket_OnError(const FString& in_error)
{
    if (m_s2sClient != nullptr)
        UE_LOG(S2SWebSocket, Log, TEXT("Error: %s"), *in_error);

    rttEnabledFailureCallback(in_error);
}

void US2SRTTComms::webSocket_OnClose()
{
    if (m_s2sClient != nullptr)
    {
        UE_LOG(S2SWebSocket, Log, TEXT("Connection closed"));

        if (m_disconnectedWithReason == true)
        {
            FString response;
            TSharedRef<TJsonWriter<>> disconnectJson = TJsonWriterFactory<>::Create(&response);
            UE_LOG(S2SWebSocket, Log, TEXT("RTT: Disconnect "), *response);
        }
    }
    if (!m_disconnectedWithReason)
    {
        disconnect();
    }
    m_disconnectedWithReason = true;
}

void US2SRTTComms::websocket_OnOpen()
{
    s2sSocket->SendText(buildConnectionRequest());
}

FString US2SRTTComms::buildConnectionRequest()
{
    FString platform = UGameplayStatics::GetPlatformName();

    // Get appId from the owning S2S context
    FString appId = m_s2sClient ? m_s2sClient->getSessionData().appId : TEXT("");

    TSharedRef<FJsonObject> sysJson = MakeShareable(new FJsonObject());
    sysJson->SetStringField(S2SOperationParam::Platform, platform);
    sysJson->SetStringField(S2SOperationParam::Protocol, TEXT("ws"));

    TSharedRef<FJsonObject> jsonData = MakeShareable(new FJsonObject());
    jsonData->SetStringField(S2SOperationParam::AppId,     appId);
    jsonData->SetStringField(S2SOperationParam::SessionId, m_s2sClient->getSessionID());
    jsonData->SetStringField(S2SOperationParam::ProfileId, TEXT("s"));
    jsonData->SetObjectField(S2SOperationParam::System,    sysJson);
    jsonData->SetObjectField(S2SOperationParam::Auth,      m_rttHeaders);

    TSharedPtr<FJsonObject> json = MakeShareable(new FJsonObject());
    json->SetStringField(TEXT("service"),   S2SServiceName::RTT);
    json->SetStringField(TEXT("operation"), S2SServiceOperation::Connect);
    json->SetObjectField(TEXT("data"),      jsonData);

    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(json.ToSharedRef(), Writer);

    return JsonString;
}

FString US2SRTTComms::buildHeartbeatRequest()
{
    TSharedRef<FJsonObject> json = MakeShareable(new FJsonObject());
    json->SetStringField(TEXT("service"),   S2SServiceName::RTT);
    json->SetStringField(TEXT("operation"), S2SServiceOperation::Heartbeat);
    json->SetObjectField(TEXT("data"),      nullptr);

    return JsonUtil::jsonValueToString(json);
}

void US2SRTTComms::registerRTTCallback(const US2SCallback& callback)
{
    if (registeredBPCallback == nullptr) {
        registeredBPCallback = callback;
    }
    else {
        UE_LOG(LogBrainCloudS2S, Warning, TEXT("A callback is already registered, deregister it first to register a new one"));
    }
}

void US2SRTTComms::deregisterRTTCallback()
{
    if (registeredBPCallback != nullptr)
        registeredBPCallback = nullptr;
}

void US2SRTTComms::processRTTCallback(const FString& in_message)
{
    if (registeredBPCallback != nullptr)
        registeredBPCallback(in_message);
}

// --------------------------------------------------------------------------
// Blueprint wrapper helper
// --------------------------------------------------------------------------

US2SCallback US2SRTTComms::WrapBPDelegate(const FS2SResponseDelegate& Delegate)
{
    FS2SResponseDelegate DelegateCopy = Delegate;
    return [DelegateCopy](const FString& JsonResponse)
    {
        // For RTT messages, determine success from the status field if present
        bool bSuccess = true;
        TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(JsonResponse);
        TSharedPtr<FJsonObject> JsonObj;
        if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj->HasField(TEXT("status")))
        {
            bSuccess = (JsonObj->GetIntegerField(TEXT("status")) == 200);
        }
        DelegateCopy.ExecuteIfBound(bSuccess, JsonResponse);
    };
}

// --------------------------------------------------------------------------
// Blueprint wrappers
// --------------------------------------------------------------------------

void US2SRTTComms::S2S_EnableRTT(const FS2SResponseDelegate& OnSuccess, const FS2SResponseDelegate& OnFailure)
{
    enableRTT(WrapBPDelegate(OnSuccess), WrapBPDelegate(OnFailure));
}

void US2SRTTComms::S2S_DisableRTT()
{
    disableRTT();
}

void US2SRTTComms::S2S_RegisterRTTCallback(const FS2SResponseDelegate& Callback)
{
    registerRTTCallback(WrapBPDelegate(Callback));
}

void US2SRTTComms::S2S_DeregisterRTTCallback()
{
    deregisterRTTCallback();
}

void US2SRTTComms::S2S_Send(const FString& Message, bool bAllowLogging)
{
    send(Message, bAllowLogging);
}

void US2SRTTComms::S2S_Disconnect()
{
    disconnect();
}
