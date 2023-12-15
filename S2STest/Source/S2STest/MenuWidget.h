// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BrainCloudS2S/S2SRTTComms.h"
#include "BrainCloudS2S/BrainCloudS2S.h"
#include "MenuWidget.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FS2SRTTCallbackDelegate, const FString&, JsonData);
/**
 * 
 */
UCLASS()
class S2STEST_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	US2SRTTComms *_bc;
	int lastServerTime = 0;

	UFUNCTION(BlueprintCallable, Category = "BrainCloudS2S")
		void RunCallbacks();

	UFUNCTION(BlueprintCallable, Category = "BrainCloudS2S")
		void TestAuthentication(FS2SRTTCallbackDelegate callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloudS2S")
		void TestEnableRTT(FS2SRTTCallbackDelegate successCallback, FS2SRTTCallbackDelegate failureCallback, FS2SRTTCallbackDelegate rttCallback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloudS2S")
		void TestRTTMessage(FS2SRTTCallbackDelegate callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloudS2S")
		void TestDisableRTT(FS2SRTTCallbackDelegate callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloudS2S")
		void TestJoinSysChannel(FS2SRTTCallbackDelegate callback);

	UFUNCTION(BlueprintCallable, Category = "JsonUtil")
		bool isError(const FString& jsonMessage);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "S2S Test")
		void LogMessage(const FString& message);


	void NativeConstruct() override;

	
	FString appId;
	int m_successCounter = 0;

};

DECLARE_LOG_CATEGORY_EXTERN(LogBrainCloudS2STest, Log, All);