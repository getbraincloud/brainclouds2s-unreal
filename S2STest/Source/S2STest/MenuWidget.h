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
	UBrainCloudS2S* _bc;
	FString bcLogs;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* LogText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* RunTestButton;

	UFUNCTION()
		void OnRunTestButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "BrainCloudS2S")
		void RunCallbacks();

	//callbacks
	void onTestCallback(const FString& jsonString);
	void onTestAuthCallback(const FString& jsonString);

	void NativeConstruct() override;

};
