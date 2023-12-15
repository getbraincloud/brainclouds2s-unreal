// Copyright 2023 bitHeads, Inc. All Rights Reserved.


#include "S2SRTTComms.h"
#include <Kismet/GameplayStatics.h>
#include "ConvertUtilities.h"

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

void US2SRTTComms::InitializeS2S(const FString& appId, const FString& serverName, const FString& serverSecret, const FString& url, bool autoAuth, bool logEnabled)
{
    // Create S2S context
    m_s2sClient = MakeShareable(new UBrainCloudS2S(appId,
        serverName,
        serverSecret,
        url, true));

    m_s2sClient->Init(appId, serverName, serverSecret, url, true);

    // Verbose log
    m_s2sClient->setLogEnabled(logEnabled);
}

void US2SRTTComms::runCallbacks()
{
    if (m_s2sClient.IsValid()) {
        m_s2sClient->runCallbacks();

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
}

void US2SRTTComms::enableRTT(const US2SCallback& OnSuccess, const US2SCallback& OnFailure)
{
    if (m_s2sClient.IsValid()) {

        TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

        JsonObject->SetStringField("service", "rttRegistration");
        JsonObject->SetStringField("operation", "REQUEST_SYSTEM_CONNECTION");

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
            TArray<TSharedPtr<FJsonValue>> endpoints = jsonData->GetArrayField(TEXT("endpoints"));
            m_rttHeaders = jsonData->GetObjectField(TEXT("auth"));

            //setup socket connection
            TSharedPtr<FJsonObject> endpoint = endpoints[0]->AsObject();
            bool sslEnabled = endpoint->GetBoolField(TEXT("ssl"));

            FString url = (sslEnabled ? TEXT("wss://") : TEXT("ws://"));
            url += endpoint->GetStringField(TEXT("host"));
            url += ":";
            url += FString::Printf(TEXT("%d"), endpoint->GetIntegerField(TEXT("port")));
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
    if (s2sSocket->IsConnected()) {
        disconnect();
    }
    deregisterRTTCallback();
}

void US2SRTTComms::request(const FString& requestJson, const US2SCallback& Callback)
{
    if (m_s2sClient.IsValid()) {
        m_s2sClient->request(requestJson, Callback);
    }
}

void US2SRTTComms::authenticate(const US2SCallback& callback)
{
    if (m_s2sClient.IsValid()) {
        m_s2sClient->authenticate(callback);
    }
}

void US2SRTTComms::authenticate()
{
    if (m_s2sClient.IsValid()) {
        m_s2sClient->authenticate();
    }
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
    if (!m_s2sClient.IsValid()) return;
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
}

void US2SRTTComms::webSocket_OnMessage(const TArray<uint8>& in_data)
{
    FString parsedMessage = ConvertUtilities::BCBytesToString(in_data.GetData(), in_data.Num());
    UE_LOG(S2SWebSocket, Log, TEXT("RECV: %s "), *parsedMessage);

    TSharedPtr<FJsonObject> jsonData = ConvertUtilities::jsonStringToValue(parsedMessage);
    FString service = jsonData->HasField(TEXT("service")) ? jsonData->GetStringField(TEXT("service")) : "";
    FString operation = jsonData->HasField(TEXT("operation")) ? jsonData->GetStringField(TEXT("operation")) : "";
    TSharedPtr<FJsonObject> innerData = nullptr;
    bool bIsInnerDataValid = jsonData->HasTypedField<EJson::Object>(TEXT("data"));

    if (bIsInnerDataValid)
        innerData = jsonData->GetObjectField(TEXT("data"));

    if (operation == "HEARTBEAT") {
        m_heartBeatRecv = true;
    }

    if (operation == "CONNECT")
    {
        int32 heartBeat = INITIAL_HEARTBEAT_TIME;
        if (bIsInnerDataValid && innerData->HasField(TEXT("heartbeatSeconds")))
        {
            heartBeat = innerData->GetIntegerField(TEXT("heartbeatSeconds"));
        }
        else if (bIsInnerDataValid && innerData->HasField(TEXT("wsHeartbeatSecs")))
        {
            heartBeat = innerData->GetIntegerField(TEXT("wsHeartbeatSecs"));
        }

        isRTTEnabled = true;
        setRTTHeartBeatSeconds(heartBeat);

        rttEnabledSuccessCallback(parsedMessage);
    }
    else if (operation == "DISCONNECT")
    {
        m_disconnectedWithReason = true;
        m_disconnectJson->SetStringField("reason", innerData->GetStringField(TEXT("reason")));
        m_disconnectJson->SetNumberField("reasonCode", innerData->GetNumberField(TEXT("reasonCode")));
        m_disconnectJson->SetStringField("severity", "ERROR");
    }

    processRTTCallback(parsedMessage);

    if (bIsInnerDataValid)
    {
        if (innerData->HasField(TEXT("cxId")))
        {
            m_cxId = innerData->GetStringField(TEXT("cxId"));
        }

        if (innerData->HasField(TEXT("evs")))
        {
            m_eventServer = innerData->GetStringField(TEXT("evs"));
        }
    }

}

void US2SRTTComms::webSocket_OnError(const FString& in_error)
{
    if (m_s2sClient.IsValid())
        UE_LOG(S2SWebSocket, Log, TEXT("Error: %s"), *in_error);

    rttEnabledFailureCallback(in_error);
}

void US2SRTTComms::webSocket_OnClose()
{
    if (m_s2sClient.IsValid())
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

    TSharedRef<FJsonObject> sysJson = MakeShareable(new FJsonObject());
    sysJson->SetStringField("platform", platform);
    sysJson->SetStringField("protocol", "ws");

    TSharedRef<FJsonObject> jsonData = MakeShareable(new FJsonObject());
    jsonData->SetStringField("appId", getenv("APP_ID"));
    jsonData->SetStringField("sessionId", m_s2sClient->getSessionID());
    jsonData->SetStringField("profileId", "s");
    jsonData->SetObjectField("system", sysJson);
    jsonData->SetObjectField("auth", m_rttHeaders);

    TSharedPtr<FJsonObject> json = MakeShareable(new FJsonObject());
    json->SetStringField("service", "RTT");
    json->SetStringField("operation", "CONNECT");
    json->SetObjectField("data", jsonData);

    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(json.ToSharedRef(), Writer);

    return JsonString;
}

FString US2SRTTComms::buildHeartbeatRequest()
{
    TSharedRef<FJsonObject> json = MakeShareable(new FJsonObject());
    json->SetStringField("service", "RTT");
    json->SetStringField("operation", "HEARTBEAT");
    json->SetObjectField("data", nullptr);

    return ConvertUtilities::jsonValueToString(json);
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
