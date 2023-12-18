// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"
#include "Runtime/UMG/Public/Components/Button.h" //unreal has a "INCLUDE WHAT YOU USE" structure for fast compilation so you need to include each class you use from their engine. 
#include "Runtime/UMG/Public/Components/TextBlock.h"
#include "BrainCloudS2S/ConvertUtilities.h"

DEFINE_LOG_CATEGORY(LogBrainCloudS2STest);

void UMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//INIT BrainCloud
	appId = "";
	FString serverName = "";
	FString serverSecret = "";
	FString url = "https://api.internal.braincloudservers.com/s2sdispatcher";

	_bc = NewObject<US2SRTTComms>();
	_bc->AddToRoot();
	_bc->InitializeS2S(appId, serverName, serverSecret, url, false, true);
}

void UMenuWidget::RunCallbacks()
{
	if (_bc != NULL)
		_bc->runCallbacks();
}

void UMenuWidget::TestAuthentication(FS2SRTTCallbackDelegate callback)
{
	if (_bc == nullptr) return;
	_bc->authenticate([this, callback](const FString& result)
		{
			callback.ExecuteIfBound(result);
		});
}

void UMenuWidget::TestEnableRTT(FS2SRTTCallbackDelegate successCallback, FS2SRTTCallbackDelegate failureCallback, FS2SRTTCallbackDelegate rttCallback)
{
	if (_bc == nullptr) return;
	_bc->enableRTT([this, successCallback](const FString& result)
		{
			successCallback.ExecuteIfBound(result);
		}, [this, failureCallback](const FString& result)
		{
			failureCallback.ExecuteIfBound(result);
		});
	//Register callback for receiving RTT message
	_bc->registerRTTCallback([this, rttCallback](const FString& result)
		{
			rttCallback.ExecuteIfBound(result);
		});
}

void UMenuWidget::TestRTTMessage(FS2SRTTCallbackDelegate callback)
{
	if (_bc == nullptr) return;
	TSharedRef<FJsonObject> requestJson = MakeShareable(new FJsonObject());
	requestJson->SetStringField("service", "chat");
	requestJson->SetStringField("operation", "SYS_POST_CHAT_MESSAGE");

	TSharedRef<FJsonObject> dataJson = MakeShareable(new FJsonObject());
	dataJson->SetStringField("channelId", appId + ":sy:mysyschannel");

	TSharedRef<FJsonObject> contentJson = MakeShareable(new FJsonObject());
	contentJson->SetStringField("foo", "bar");
	contentJson->SetStringField("someData", "12345");

	dataJson->SetObjectField("content", contentJson);
	dataJson->SetBoolField("recordInHistory", false);
	
	requestJson->SetObjectField("data", dataJson);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(requestJson, Writer);

	_bc->request(JsonString, [this, callback](const FString& result)
		{
			callback.ExecuteIfBound(result);
		});
}

void UMenuWidget::TestDisableRTT(FS2SRTTCallbackDelegate callback)
{
	if (_bc == nullptr) return;

	_bc->disableRTT();
	callback.ExecuteIfBound("[RTT Disabled]");
}

void UMenuWidget::TestJoinSysChannel(FS2SRTTCallbackDelegate callback)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	JsonObject->SetStringField("service", "chat");
	JsonObject->SetStringField("operation", "SYS_CHANNEL_CONNECT");

	TSharedPtr<FJsonObject> dataObject = MakeShareable(new FJsonObject);
	dataObject->SetStringField("channelId", appId + ":sy:mysyschannel");
	dataObject->SetNumberField("maxReturn", 0);

	JsonObject->SetObjectField("data", dataObject);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	_bc->request(JsonString,
		[this, callback](const FString& result)
		{
			callback.ExecuteIfBound(result);
		});

	LogMessage("Sent message: " + JsonString);
}

bool UMenuWidget::isError(const FString& jsonMessage)
{
	TSharedPtr<FJsonObject> jsonData = ConvertUtilities::jsonStringToValue(jsonMessage);
	int status = jsonData->HasField(TEXT("status")) ? jsonData->GetNumberField(TEXT("status")) : 200;

	return status != 200;
}

void UMenuWidget::LogMessage_Implementation(const FString& message)
{
	//Implement in blueprint
}
