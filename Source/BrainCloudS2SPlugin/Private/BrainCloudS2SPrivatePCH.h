// Copyright 2024 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Delegates/Delegate.h"
//#include "Http.h"
#include "Containers/Map.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(S2SWebSocket, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogBrainCloudS2S, Log, All);

#include "BrainCloudS2SPlugin.h"

#include "BrainCloudS2SPluginClasses.h"
