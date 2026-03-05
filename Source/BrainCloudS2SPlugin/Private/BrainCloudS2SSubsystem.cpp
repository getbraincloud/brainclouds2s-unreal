// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#include "BrainCloudS2SSubsystem.h"
#include "BrainCloudS2S.h"

void UBrainCloudS2SSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UBrainCloudS2SSubsystem::Deinitialize()
{
    if (S2SContext)
    {
        S2SContext->disconnect();
        S2SContext = nullptr;
    }
    Super::Deinitialize();
}

void UBrainCloudS2SSubsystem::Tick(float DeltaTime)
{
    if (S2SContext)
    {
        S2SContext->runCallbacks();
    }
}

TStatId UBrainCloudS2SSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UBrainCloudS2SSubsystem, STATGROUP_Tickables);
}

bool UBrainCloudS2SSubsystem::IsTickable() const
{
    return S2SContext != nullptr;
}

void UBrainCloudS2SSubsystem::InitializeS2S(
    const FString& AppId,
    const FString& ServerName,
    const FString& ServerSecret,
    const FString& Url,
    bool bAutoAuth)
{
    if (S2SContext)
    {
        S2SContext->disconnect();
    }

    S2SContext = UBrainCloudS2S::CreateS2SContext(AppId, ServerName, ServerSecret, Url, bAutoAuth);
}

UBrainCloudS2S* UBrainCloudS2SSubsystem::GetS2SContext() const
{
    return S2SContext;
}
