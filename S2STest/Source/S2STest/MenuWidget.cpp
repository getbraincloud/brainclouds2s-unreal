// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"
#include "Runtime/UMG/Public/Components/Button.h" //unreal has a "INCLUDE WHAT YOU USE" structure for fast compilation so you need to include each class you use from their engine. 
#include "Runtime/UMG/Public/Components/TextBlock.h"

DEFINE_LOG_CATEGORY(LogBrainCloudS2STest);

void UMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//INIT BrainCloud
	FString appId = "20001";
	FString serverName = "TestServer";
	FString serverSecret = "2ddf8355-c516-48dd-a6b0-e35bd75fac80";
	FString url = "https://internal.braincloudservers.com/s2sdispatcher";

	_bc = MakeShareable(new UBrainCloudS2S("20001", "TestServer", "2ddf8355-c516-48dd-a6b0-e35bd75fac80", "https://sharedprod.braincloudservers.com/s2sdispatcher", false));
	_bc->Init(appId, serverName, serverSecret, url, false);
	_bc->setLogEnabled(true);

	//Handle Button clicks
	RunTestButton->OnClicked.AddUniqueDynamic(this, &UMenuWidget::OnRunTestButtonClicked);
}

void UMenuWidget::OnRunTestButtonClicked()
{
	//Begin tests 
	//TEST 1 - Test Auth
	_bc->authenticate(std::bind(&UMenuWidget::onTestAuthCallback, this, std::placeholders::_1));
}

void UMenuWidget::RunCallbacks()
{
	if (_bc != NULL)
		_bc->runCallbacks();
}

void UMenuWidget::onTestAuthCallback(const FString& jsonString)
{
	m_successCounter++;
	if (m_successCounter == 1)
	{
		UE_LOG(LogBrainCloudS2STest, Log, TEXT("Test1 - Auth Test Pass"));
		bcLogs += "\nTest1 - Auth Test Pass";
		LogText->SetText(FText::FromString(bcLogs));
		m_successCounter = 0; //reset
		_bc->disconnect();

		//TEST 2 - Test Multiple Auth 
		_bc->authenticate(std::bind(&UMenuWidget::onTestMultipleAuthCallback, this, std::placeholders::_1));
		_bc->authenticate(std::bind(&UMenuWidget::onTestMultipleAuthCallback, this, std::placeholders::_1));
	}
}

//tests
void UMenuWidget::onTestMultipleAuthCallback(const FString& jsonString)
{
	m_successCounter++;
	if (m_successCounter == 2)
	{
		UE_LOG(LogBrainCloudS2STest, Log, TEXT("\nTest2 - Multiple Auth PASS"));
		bcLogs += "\nTest2 - Multiple Auth PASS";
		LogText->SetText(FText::FromString(bcLogs));
		m_successCounter = 0; //reset
		_bc->disconnect();

		//TEST 3 - Test Auth and Request
		_bc->authenticate(std::bind(&UMenuWidget::onTestAuthAndRequestCallback, this, std::placeholders::_1));
		_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestAuthAndRequestCallback, this, std::placeholders::_1));
	}
}


void UMenuWidget::onTestAuthAndRequestCallback(const FString& jsonString)
{
	m_successCounter++;
	if (m_successCounter == 2)
	{
		UE_LOG(LogBrainCloudS2STest, Log, TEXT("\nTest3 - Auth and Request PASS"));
		bcLogs += "\nTest3 - Auth and Request PASS";
		LogText->SetText(FText::FromString(bcLogs));
		m_successCounter = 0; //reset
		_bc->disconnect();

		//TEST 4 - Test Auth With Null and Request
		_bc->authenticate();
		_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestNullAuthAndRequestCallback, this, std::placeholders::_1));
	}
}

void UMenuWidget::onTestNullAuthAndRequestCallback(const FString& jsonString)
{
	m_successCounter++;
	if (m_successCounter == 1)
	{
		UE_LOG(LogBrainCloudS2STest, Log, TEXT("\nTest4 - Auth With Null and Request PASS"));
		bcLogs += "\nTest4 - Auth With Null and Request PASS";
		LogText->SetText(FText::FromString(bcLogs));
		m_successCounter = 0; //reset
		_bc->disconnect();

		//TEST 5 - Test queue
		_bc->authenticate();
		_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestQueueCallback, this, std::placeholders::_1));
		_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestQueueCallback, this, std::placeholders::_1));
		_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestQueueCallback, this, std::placeholders::_1));
	}
}

void UMenuWidget::onTestQueueCallback(const FString& jsonString)
{
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonString);
	TSharedPtr<FJsonObject> jsonValue;
	FJsonSerializer::Deserialize(reader, jsonValue);
	TSharedPtr<FJsonObject> dataObject = jsonValue.Get()->GetObjectField("data");
	FString serverTime = dataObject.Get()->GetStringField("server_time");
	UE_LOG(LogBrainCloudS2STest, Log, TEXT("serverTime - %s"), *serverTime);

	//check if serverTime is greater than stored server time - this way we know later requests are coming back last in the queue
	if (FCString::Atoi(*serverTime) >= lastServerTime)
	{
		m_successCounter++;
		UE_LOG(LogBrainCloudS2STest, Log, TEXT("success - %d"), m_successCounter);
		lastServerTime = FCString::Atoi(*serverTime);
	}

	//when all three calls come in, and have come in properly queued, then it's a success!
	if (m_successCounter == 3)
	{
		UE_LOG(LogBrainCloudS2STest, Log, TEXT("\nTest5 - Queue PASS"));
		bcLogs += "\nTest5 - Queue PASS";
		LogText->SetText(FText::FromString(bcLogs));
		m_successCounter = 0; //reset
		lastServerTime = 0; //reset
		_bc->disconnect();

		//TEST 6 - Test queue with fail 
		_bc->authenticate();
		_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestQueueWithFailCallback, this, std::placeholders::_1));
		_bc->request("{\"service\":\"time\",\"operation\":\"REAAAD\"}", std::bind(&UMenuWidget::onTestQueueWithFailCallback, this, std::placeholders::_1));
		_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestQueueWithFailCallback, this, std::placeholders::_1));
	}
}

void UMenuWidget::onTestQueueWithFailCallback(const FString& jsonString)
{
	TSharedPtr<FJsonObject> jsonValue;
	TSharedPtr<FJsonObject> dataObject;
	FString serverTime;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonString);
	FJsonSerializer::Deserialize(reader, jsonValue);
	const TSharedPtr<FJsonObject>* checkData;
	if (jsonValue.Get()->TryGetObjectField(TEXT("data"), checkData))
	{
		dataObject = jsonValue.Get()->GetObjectField("data");
		serverTime = dataObject.Get()->GetStringField("server_time");
		UE_LOG(LogBrainCloudS2STest, Log, TEXT("serverTime - %s"), *serverTime);

		//check if serverTime is greater than stored server time - this way we know later requests are coming back last in the queue
		if (FCString::Atoi(*serverTime) >= lastServerTime)
		{
			m_successCounter++;
			UE_LOG(LogBrainCloudS2STest, Log, TEXT("success - %d"), m_successCounter);
			lastServerTime = FCString::Atoi(*serverTime);
		}
	}

	//Expecting 2 calls to pass and the the second call to fail but not break the queue
	if (m_successCounter == 2)
	{
		UE_LOG(LogBrainCloudS2STest, Log, TEXT("\nTest6 - Queue With Fail PASS"));
		bcLogs += "\nTest6 - Queue With Fail PASS";
		LogText->SetText(FText::FromString(bcLogs));
		m_successCounter = 0; //reset
		lastServerTime = 0; //reset
		_bc->disconnect();

		//TEST 7 - Test queue with fail 
		_bc->authenticate();
		//4 working calls
		_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestRandomFailureCallback, this, std::placeholders::_1)); //good
		_bc->request("{\"service\":\"time\",\"operation\":\"REAAAD\"}", std::bind(&UMenuWidget::onTestRandomFailureCallback, this, std::placeholders::_1)); //bad
		_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestRandomFailureCallback, this, std::placeholders::_1)); //good
		_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestRandomFailureCallback, this, std::placeholders::_1)); //good
		_bc->request("{\"service\":\"time\",\"operation\":\"REAAAD\"}", std::bind(&UMenuWidget::onTestRandomFailureCallback, this, std::placeholders::_1)); //bad
		_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestRandomFailureCallback, this, std::placeholders::_1)); //good 
	}
}

void UMenuWidget::onTestRandomFailureCallback(const FString& jsonString)
{
	TSharedPtr<FJsonObject> jsonValue;
	TSharedPtr<FJsonObject> dataObject;
	FString serverTime;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonString);
	FJsonSerializer::Deserialize(reader, jsonValue);
	const TSharedPtr<FJsonObject>* checkData;
	if (jsonValue.Get()->TryGetObjectField(TEXT("data"), checkData))
	{
		dataObject = jsonValue.Get()->GetObjectField("data");
		serverTime = dataObject.Get()->GetStringField("server_time");
		UE_LOG(LogBrainCloudS2STest, Log, TEXT("serverTime - %s"), *serverTime);

		//check if serverTime is greater than stored server time - this way we know later requests are coming back last in the queue
		if (FCString::Atoi(*serverTime) >= lastServerTime)
		{
			m_successCounter++;
			UE_LOG(LogBrainCloudS2STest, Log, TEXT("success - %d"), m_successCounter);
			lastServerTime = FCString::Atoi(*serverTime);
		}
	}

	if (m_successCounter == 4)
	{
		UE_LOG(LogBrainCloudS2STest, Log, TEXT("\nTest7 - Random Fail PASS"));
		bcLogs += "\nTest7 - Random Fail PASS";
		LogText->SetText(FText::FromString(bcLogs));
		m_successCounter = 0; //reset
		_bc->disconnect();
	}
}