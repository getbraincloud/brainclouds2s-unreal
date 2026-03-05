// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BrainCloudS2STypes.generated.h"

/**
 * Dynamic delegate for Blueprint-compatible S2S response callbacks.
 *
 * @param bSuccess   true when the server returned HTTP 200 with a valid response.
 * @param JsonResponse  The JSON response string from the server.
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FS2SResponseDelegate, bool, bSuccess, const FString&, JsonResponse);

/** Placeholder struct so UHT processes this header and registers the delegate. */
USTRUCT()
struct FS2STypes
{
	GENERATED_BODY()
};
