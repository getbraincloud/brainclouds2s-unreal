// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * String constants for all S2S service names.
 * Use these instead of raw string literals when calling request() or
 * building JSON manually.
 */
namespace S2SServiceName
{
    // Core
    static constexpr const TCHAR* AuthenticationV2 = TEXT("authenticationV2");
    static constexpr const TCHAR* Heartbeat        = TEXT("heartbeat");

    // Real-Time Tech (RTT)
    static constexpr const TCHAR* RTTRegistration = TEXT("rttRegistration");
    static constexpr const TCHAR* RTT             = TEXT("RTT");

    // Services
    static constexpr const TCHAR* GlobalFileV3 = TEXT("globalFileV3");
    static constexpr const TCHAR* Chat         = TEXT("chat");
}
