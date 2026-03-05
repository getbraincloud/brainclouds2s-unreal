// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

/**
 * Lightweight helpers for building the inner S2S message JSON:
 *
 *   {"service":"<svc>","operation":"<op>"}
 *   {"service":"<svc>","operation":"<op>","data":{...}}
 *
 * Use S2SServiceName / S2SServiceOperation / S2SOperationParam constants
 * together with these functions to avoid raw string literals.
 */
namespace S2SRequestBuilder
{
    /** Build a message with no data object. */
    inline FString Build(const TCHAR* Service, const TCHAR* Operation)
    {
        TSharedRef<FJsonObject> Request = MakeShared<FJsonObject>();
        Request->SetStringField(TEXT("service"),   Service);
        Request->SetStringField(TEXT("operation"), Operation);

        FString Output;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
        FJsonSerializer::Serialize(Request, Writer);
        return Output;
    }

    /** Build a message with a pre-populated data object. */
    inline FString Build(const TCHAR* Service, const TCHAR* Operation,
                         const TSharedRef<FJsonObject>& Data)
    {
        TSharedRef<FJsonObject> Request = MakeShared<FJsonObject>();
        Request->SetStringField(TEXT("service"),   Service);
        Request->SetStringField(TEXT("operation"), Operation);
        Request->SetObjectField(TEXT("data"),      Data);

        FString Output;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
        FJsonSerializer::Serialize(Request, Writer);
        return Output;
    }
}
