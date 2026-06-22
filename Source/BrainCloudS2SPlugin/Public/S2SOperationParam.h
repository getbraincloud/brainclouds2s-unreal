// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * String constants for JSON field/parameter names used in S2S request data objects.
 */
namespace S2SOperationParam
{
    // -----------------------------------------------------------------------
    // Envelope / session
    // -----------------------------------------------------------------------
    static constexpr const TCHAR* PacketId         = TEXT("packetId");
    static constexpr const TCHAR* SessionId        = TEXT("sessionId");
    static constexpr const TCHAR* Messages         = TEXT("messages");
    static constexpr const TCHAR* MessageResponses = TEXT("messageResponses");
    static constexpr const TCHAR* Status           = TEXT("status");
    static constexpr const TCHAR* ReasonCode       = TEXT("reason_code");

    // -----------------------------------------------------------------------
    // Authentication
    // -----------------------------------------------------------------------
    static constexpr const TCHAR* AppId        = TEXT("appId");
    static constexpr const TCHAR* ServerName   = TEXT("serverName");
    static constexpr const TCHAR* ServerSecret = TEXT("serverSecret");

    // -----------------------------------------------------------------------
    // RTT / WebSocket
    // -----------------------------------------------------------------------
    static constexpr const TCHAR* Platform          = TEXT("platform");
    static constexpr const TCHAR* Protocol          = TEXT("protocol");
    static constexpr const TCHAR* ProfileId         = TEXT("profileId");
    static constexpr const TCHAR* System            = TEXT("system");
    static constexpr const TCHAR* Auth              = TEXT("auth");
    static constexpr const TCHAR* Endpoints        = TEXT("endpoints");
    static constexpr const TCHAR* Ssl               = TEXT("ssl");
    static constexpr const TCHAR* Host              = TEXT("host");
    static constexpr const TCHAR* Port              = TEXT("port");
    static constexpr const TCHAR* CxId              = TEXT("cxId");
    static constexpr const TCHAR* Evs               = TEXT("evs");
    static constexpr const TCHAR* HeartbeatSeconds  = TEXT("heartbeatSeconds");
    static constexpr const TCHAR* WsHeartbeatSecs   = TEXT("wsHeartbeatSecs");
    static constexpr const TCHAR* Reason            = TEXT("reason");
    static constexpr const TCHAR* Severity          = TEXT("severity");

    // -----------------------------------------------------------------------
    // globalFileV3 — common
    // -----------------------------------------------------------------------
    static constexpr const TCHAR* FileId              = TEXT("fileId");
    static constexpr const TCHAR* FolderPath          = TEXT("folderPath");
    static constexpr const TCHAR* Filename            = TEXT("filename");
    static constexpr const TCHAR* FullPathFilename    = TEXT("fullPathFilename");
    static constexpr const TCHAR* Recurse             = TEXT("recurse");
    static constexpr const TCHAR* OverwriteIfPresent  = TEXT("overwriteIfPresent");
    static constexpr const TCHAR* Version             = TEXT("version");
    static constexpr const TCHAR* TreeId              = TEXT("treeId");
    static constexpr const TCHAR* TreeVersion         = TEXT("treeVersion");
    static constexpr const TCHAR* NewTreeId           = TEXT("newTreeId");
    static constexpr const TCHAR* NewFilename         = TEXT("newFilename");
    static constexpr const TCHAR* FileSize            = TEXT("fileSize");
    static constexpr const TCHAR* FileDetails         = TEXT("fileDetails");
    static constexpr const TCHAR* UploadId            = TEXT("uploadId");
    static constexpr const TCHAR* UploadUrl           = TEXT("uploadUrl");

    // globalFileV3 — user file move
    static constexpr const TCHAR* UserProfileId    = TEXT("userProfileId");
    static constexpr const TCHAR* UserCloudPath    = TEXT("userCloudPath");
    static constexpr const TCHAR* UserCloudFilename = TEXT("userCloudFilename");
    static constexpr const TCHAR* GlobalTreeId     = TEXT("globalTreeId");
    static constexpr const TCHAR* GlobalFilename   = TEXT("globalFilename");

    // globalFileV3 — folder management
    static constexpr const TCHAR* Name                     = TEXT("name");
    static constexpr const TCHAR* Desc                     = TEXT("desc");
    static constexpr const TCHAR* CreateInterimDirectories = TEXT("createInterimDirectories");
    static constexpr const TCHAR* NewFolderPath            = TEXT("newFolderPath");
    static constexpr const TCHAR* UpdatedName              = TEXT("updatedName");
    static constexpr const TCHAR* FullFolderPath           = TEXT("fullFolderPath");
    static constexpr const TCHAR* Force                    = TEXT("force");
    static constexpr const TCHAR* CreatedTreeId            = TEXT("createdTreeId");

    // -----------------------------------------------------------------------
    // Chat
    // -----------------------------------------------------------------------
    static constexpr const TCHAR* ChannelId       = TEXT("channelId");
    static constexpr const TCHAR* MaxReturn       = TEXT("maxReturn");
    static constexpr const TCHAR* Content         = TEXT("content");
    static constexpr const TCHAR* RecordInHistory = TEXT("recordInHistory");
}
