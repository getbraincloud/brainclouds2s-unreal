// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * String constants for all S2S operation names.
 * Grouped by the service they belong to.
 */
namespace S2SServiceOperation
{
    // authenticationV2
    static constexpr const TCHAR* Authenticate = TEXT("AUTHENTICATE");

    // heartbeat  /  RTT (both use the same wire string)
    static constexpr const TCHAR* Heartbeat = TEXT("HEARTBEAT");

    // rttRegistration
    static constexpr const TCHAR* RequestSystemConnection = TEXT("REQUEST_SYSTEM_CONNECTION");

    // RTT (websocket layer)
    static constexpr const TCHAR* Connect    = TEXT("CONNECT");
    static constexpr const TCHAR* Disconnect = TEXT("DISCONNECT");

    // globalFileV3 — file info / query
    static constexpr const TCHAR* SysGetFileInfo                  = TEXT("SYS_GET_FILE_INFO");
    static constexpr const TCHAR* SysGetFileInfoSimple            = TEXT("SYS_GET_FILE_INFO_SIMPLE");
    static constexpr const TCHAR* SysCheckFilenameExists          = TEXT("SYS_CHECK_FILENAME_EXISTS");
    static constexpr const TCHAR* SysCheckFullpathFilenameExists  = TEXT("SYS_CHECK_FULLPATH_FILENAME_EXISTS");
    static constexpr const TCHAR* SysGetGlobalCDNUrl              = TEXT("SYS_GET_GLOBAL_CDN_URL");
    static constexpr const TCHAR* SysGetGlobalFileList            = TEXT("SYS_GET_GLOBAL_FILE_LIST");

    // globalFileV3 — file management
    static constexpr const TCHAR* SysMoveToGlobalFile = TEXT("SYS_MOVE_TO_GLOBAL_FILE");
    static constexpr const TCHAR* SysCopyGlobalFile   = TEXT("SYS_COPY_GLOBAL_FILE");
    static constexpr const TCHAR* SysMoveGlobalFile   = TEXT("SYS_MOVE_GLOBAL_FILE");
    static constexpr const TCHAR* SysDeleteGlobalFile  = TEXT("SYS_DELETE_GLOBAL_FILE");
    static constexpr const TCHAR* SysDeleteGlobalFiles = TEXT("SYS_DELETE_GLOBAL_FILES");
    static constexpr const TCHAR* SysPrepareUpload     = TEXT("SYS_PREPARE_UPLOAD");

    // globalFileV3 — folder management
    static constexpr const TCHAR* SysCreateFolder = TEXT("SYS_CREATE_FOLDER");
    static constexpr const TCHAR* SysMoveFolder   = TEXT("SYS_MOVE_FOLDER");
    static constexpr const TCHAR* SysRenameFolder = TEXT("SYS_RENAME_FOLDER");
    static constexpr const TCHAR* SysLookupFolder = TEXT("SYS_LOOKUP_FOLDER");
    static constexpr const TCHAR* SysDeleteFolder = TEXT("SYS_DELETE_FOLDER");

    // chat
    static constexpr const TCHAR* SysChannelConnect  = TEXT("SYS_CHANNEL_CONNECT");
    static constexpr const TCHAR* SysPostChatMessage = TEXT("SYS_POST_CHAT_MESSAGE");
}
