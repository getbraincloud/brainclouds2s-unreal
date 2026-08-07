// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "BrainCloudS2S.h"
#include "HttpModule.h"
#include "HttpManager.h"

/**
 * Shared helpers for all BrainCloudS2S automation tests.
 *
 * Credentials are read from environment variables so the same test binary
 * works against both internal and prod environments without recompiling:
 *
 *   BC_APP_ID         — brainCloud app ID (e.g. "12011" for prod)
 *   BC_SERVER_NAME    — S2S server name   (e.g. "TestServer")
 *   BC_SERVER_SECRET  — S2S server secret
 *   BC_S2S_URL        — dispatcher URL    (e.g. "https://api.braincloudservers.com/s2sdispatcher")
 *
 * The bccm test hook sets these from data/test_ids_<env>.txt before running.
 */
namespace S2STestFixture
{
    inline FString AppId()        { return FPlatformMisc::GetEnvironmentVariable(TEXT("BC_APP_ID")); }
    inline FString ServerName()   { return FPlatformMisc::GetEnvironmentVariable(TEXT("BC_SERVER_NAME")); }
    inline FString ServerSecret() { return FPlatformMisc::GetEnvironmentVariable(TEXT("BC_SERVER_SECRET")); }
    inline FString ServerUrl()    { return FPlatformMisc::GetEnvironmentVariable(TEXT("BC_S2S_URL")); }

    inline bool CredentialsLoaded()
    {
        return !AppId().IsEmpty() && !ServerName().IsEmpty()
            && !ServerSecret().IsEmpty() && !ServerUrl().IsEmpty();
    }

    /**
     * Issues a call and blocks (pumping runCallbacks) until the response arrives
     * or 30 seconds elapse. Returns the JSON response string; empty on timeout.
     */
    inline FString RunCall(UBrainCloudS2S* Ctx, TFunction<void(US2SCallback)> Call)
    {
        bool bDone = false;
        FString Result;
        Call([&](const FString& Json)
        {
            Result = Json;
            bDone = true;
        });

        const double Deadline = FPlatformTime::Seconds() + 30.0;
        while (!bDone && FPlatformTime::Seconds() < Deadline)
        {
            // Tick the HTTP module so pending responses are dispatched on the
            // game thread — required in commandlet mode where the engine loop
            // is not running continuously.
            FHttpModule::Get().GetHttpManager().Tick(0.05f);
            Ctx->runCallbacks();
            FPlatformProcess::Sleep(0.05f);
        }
        return Result;
    }

    // Returns the top-level "status" integer from a JSON response, or 0 on parse failure.
    inline int32 GetStatus(const FString& Json)
    {
        TSharedPtr<FJsonObject> Obj;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
        if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid())
            return 0;
        double Status = 0.0;
        Obj->TryGetNumberField(TEXT("status"), Status);
        return (int32)Status;
    }

    // Returns the value of response.data.<FieldName> as an FString, or empty on failure.
    inline FString GetDataStringField(const FString& Json, const FString& FieldName)
    {
        TSharedPtr<FJsonObject> Obj;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
        if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid())
            return TEXT("");
        const TSharedPtr<FJsonObject>* Data;
        if (!Obj->TryGetObjectField(TEXT("data"), Data))
            return TEXT("");
        FString Out;
        (*Data)->TryGetStringField(FieldName, Out);
        return Out;
    }
}

#endif // WITH_DEV_AUTOMATION_TESTS
