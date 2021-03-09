// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrainCloudS2S.h"
#include "Blueprint/UserWidget.h"
#include "MenuWidget.generated.h"


/**
 * 
 */
UCLASS()
class S2STEST_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	//UBrainCloudS2S* _bc;
	TSharedPtr<UBrainCloudS2S> _bc;
	FString bcLogs;
	int lastServerTime = 0;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* LogText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* RunTestButton;

	UFUNCTION()
		void OnRunTestButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "BrainCloudS2S")
		void RunCallbacks();

	//tests as callbacks
	void onTestAuthCallback(const FString& jsonString);
	void onTestMultipleAuthCallback(const FString& jsonString);
	void onTestAuthAndRequestCallback(const FString& jsonString);
	void onTestNullAuthAndRequestCallback(const FString& jsonString);
	void onTestQueueCallback(const FString& jsonString);
	void onTestQueueWithFailCallback(const FString& jsonString);
	void onTestRandomFailureCallback(const FString& jsonString);

	void startWaitTimer(int timeSeconds);

	void NativeConstruct() override;

	int m_successCounter = 0;

};

DECLARE_LOG_CATEGORY_EXTERN(LogBrainCloudS2STest, Log, All);