// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"
#include "Runtime/UMG/Public/Components/Button.h" //unreal has a "INCLUDE WHAT YOU USE" structure for fast compilation so you need to include each class you use from their engine. 
#include "Runtime/UMG/Public/Components/TextBlock.h"

void UMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//INIT BrainCloud
	FString appId = "20001";
	FString serverName = "TestServer";
	FString serverSecret = "2ddf8355-c516-48dd-a6b0-e35bd75fac80";
	FString url = "https://internal.braincloudservers.com/s2sdispatcher";

	//TSharedPtr<UBrainCloudS2S> _bc = MakeShareable(new UBrainCloudS2S("20001", "TestServer", "2ddf8355-c516-48dd-a6b0-e35bd75fac80" ));
	//_bc = CreateDefaultSubobject<UBrainCloudS2S>(TEXT("BrainCloud"));
	_bc = NewObject<UBrainCloudS2S>();
	_bc->Init(appId, serverName, serverSecret, url, false);
	_bc->setLogEnabled(true);
	_bc->authenticate(NULL);
	_bc->authenticate(NULL);
	_bc->authenticate(NULL);
	_bc->authenticate(NULL);
	_bc->authenticate(std::bind(&UMenuWidget::onTestAuthCallback, this, std::placeholders::_1));
	_bc->authenticate(std::bind(&UMenuWidget::onTestAuthCallback, this, std::placeholders::_1));
	_bc->authenticate(std::bind(&UMenuWidget::onTestAuthCallback, this, std::placeholders::_1));
	_bc->authenticate(std::bind(&UMenuWidget::onTestAuthCallback, this, std::placeholders::_1));
	_bc->authenticate(std::bind(&UMenuWidget::onTestAuthCallback, this, std::placeholders::_1));

	//Handle Button clicks
	RunTestButton->OnClicked.AddUniqueDynamic(this, &UMenuWidget::OnRunTestButtonClicked);
}

void UMenuWidget::OnRunTestButtonClicked()
{
	//will automatically authenticate for the user
	//_bc->authenticate();
	_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestCallback, this, std::placeholders::_1));
	_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestCallback, this, std::placeholders::_1));
	_bc->request("{\"service\":\"time\",\"operation\":\"READ\"}", std::bind(&UMenuWidget::onTestCallback, this, std::placeholders::_1));
}

void UMenuWidget::RunCallbacks()
{
	if(_bc != NULL)
	_bc->runCallbacks();
}

void UMenuWidget::onTestCallback(const FString& jsonString)
{
	bcLogs += jsonString;
	//bcLogs += "\n Callback hit!";
	LogText->SetText(FText::FromString(bcLogs));
}

void UMenuWidget::onTestAuthCallback(const FString& jsonString)
{
	//bcLogs += jsonString;
	bcLogs += "\n Authenticated!";
	LogText->SetText(FText::FromString(bcLogs));
}