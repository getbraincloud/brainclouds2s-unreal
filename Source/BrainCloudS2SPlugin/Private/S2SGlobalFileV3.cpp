// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#include "S2SGlobalFileV3.h"
#include "BrainCloudS2S.h"
#include "S2SServiceName.h"
#include "S2SServiceOperation.h"
#include "S2SOperationParam.h"
#include "S2SRequestBuilder.h"
#include "Http.h"
#include "Json.h"


US2SGlobalFileV3::US2SGlobalFileV3() : _generation(0)
{
}

void US2SGlobalFileV3::SetS2SContext(UBrainCloudS2S* context)
{
    _s2s = context;
}

void US2SGlobalFileV3::init(const FString& serverUrl)
{
    const FString Needle = TEXT("s2sdispatcher");
    int32 Pos = serverUrl.Find(Needle);
    if (Pos != INDEX_NONE)
    {
        _uploadUrl = serverUrl.Left(Pos)
            + TEXT("s2suploader/globalfile/upload")
            + serverUrl.RightChop(Pos + Needle.Len());
    }
    else
    {
        _uploadUrl = TEXT("https://api.braincloudservers.com/s2suploader/globalfile/upload");
    }
}

// --------------------------------------------------------------------------
// File Info / Query
// --------------------------------------------------------------------------

void US2SGlobalFileV3::sysGetFileInfo(const FString& fileId, const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("fileId"), fileId);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysGetFileInfo, Data), callback);
}

void US2SGlobalFileV3::sysGetFileInfoSimple(const FString& folderPath, const FString& filename,
    const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("folderPath"), folderPath);
    Data->SetStringField(TEXT("filename"), filename);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysGetFileInfoSimple, Data), callback);
}

void US2SGlobalFileV3::sysCheckFilenameExists(const FString& folderPath, const FString& filename,
    const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("folderPath"), folderPath);
    Data->SetStringField(TEXT("filename"), filename);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysCheckFilenameExists, Data), callback);
}

void US2SGlobalFileV3::sysCheckFullpathFilenameExists(const FString& fullpathFilename,
    const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("fullPathFilename"), fullpathFilename);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysCheckFullpathFilenameExists, Data), callback);
}

void US2SGlobalFileV3::sysGetGlobalCDNUrl(const FString& fileId, const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("fileId"), fileId);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysGetGlobalCDNUrl, Data), callback);
}

void US2SGlobalFileV3::sysGetGlobalFileList(const FString& folderPath, bool recurse,
    const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("folderPath"), folderPath);
    Data->SetBoolField(TEXT("recurse"), recurse);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysGetGlobalFileList, Data), callback);
}

// --------------------------------------------------------------------------
// File Management
// --------------------------------------------------------------------------

void US2SGlobalFileV3::sysMoveToGlobalFile(const FString& userProfileId, const FString& userCloudPath,
    const FString& userCloudFilename, const FString& globalTreeId,
    const FString& globalFilename, bool overwriteIfPresent,
    const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("userProfileId"), userProfileId);
    Data->SetStringField(TEXT("userCloudPath"), userCloudPath);
    Data->SetStringField(TEXT("userCloudFilename"), userCloudFilename);
    Data->SetStringField(TEXT("globalTreeId"), globalTreeId);
    Data->SetStringField(TEXT("globalFilename"), globalFilename);
    Data->SetBoolField(TEXT("overwriteIfPresent"), overwriteIfPresent);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysMoveToGlobalFile, Data), callback);
}

void US2SGlobalFileV3::sysCopyGlobalFile(const FString& fileId, int version,
    const FString& newTreeId, int treeVersion,
    const FString& newFilename, bool overwriteIfPresent,
    const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("fileId"), fileId);
    Data->SetNumberField(TEXT("version"), version);
    Data->SetStringField(TEXT("newTreeId"), newTreeId);
    Data->SetNumberField(TEXT("treeVersion"), treeVersion);
    Data->SetStringField(TEXT("newFilename"), newFilename);
    Data->SetBoolField(TEXT("overwriteIfPresent"), overwriteIfPresent);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysCopyGlobalFile, Data), callback);
}

void US2SGlobalFileV3::sysMoveGlobalFile(const FString& fileId, int version,
    const FString& newTreeId, int treeVersion,
    const FString& newFilename, bool overwriteIfPresent,
    const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("fileId"), fileId);
    Data->SetNumberField(TEXT("version"), version);
    Data->SetStringField(TEXT("newTreeId"), newTreeId);
    Data->SetNumberField(TEXT("treeVersion"), treeVersion);
    Data->SetStringField(TEXT("newFilename"), newFilename);
    Data->SetBoolField(TEXT("overwriteIfPresent"), overwriteIfPresent);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysMoveGlobalFile, Data), callback);
}

void US2SGlobalFileV3::sysDeleteGlobalFile(const FString& fileId, int version,
    const FString& filename, const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("fileId"), fileId);
    Data->SetNumberField(TEXT("version"), version);
    Data->SetStringField(TEXT("filename"), filename);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysDeleteGlobalFile, Data), callback);
}

void US2SGlobalFileV3::sysDeleteGlobalFiles(const FString& treeId, const FString& folderPath,
    int treeVersion, bool recurse, const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("treeId"), treeId);
    Data->SetStringField(TEXT("folderPath"), folderPath);
    Data->SetNumberField(TEXT("treeVersion"), treeVersion);
    Data->SetBoolField(TEXT("recurse"), recurse);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysDeleteGlobalFiles, Data), callback);
}

// --------------------------------------------------------------------------
// Folder Management
// --------------------------------------------------------------------------

void US2SGlobalFileV3::sysCreateFolder(const FString& folderPath, int treeVersion,
    const FString& name, const FString& desc,
    bool createInterimDirectories, const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("folderPath"), folderPath);
    Data->SetNumberField(TEXT("treeVersion"), treeVersion);
    Data->SetStringField(TEXT("name"), name);
    Data->SetStringField(TEXT("desc"), desc);
    Data->SetBoolField(TEXT("createInterimDirectories"), createInterimDirectories);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysCreateFolder, Data), callback);
}

void US2SGlobalFileV3::sysMoveFolder(const FString& treeId, int treeVersion,
    const FString& newFolderPath, const FString& updatedName,
    bool createInterimDirectories, const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("treeId"), treeId);
    Data->SetNumberField(TEXT("treeVersion"), treeVersion);
    Data->SetStringField(TEXT("newFolderPath"), newFolderPath);
    Data->SetStringField(TEXT("updatedName"), updatedName);
    Data->SetBoolField(TEXT("createInterimDirectories"), createInterimDirectories);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysMoveFolder, Data), callback);
}

void US2SGlobalFileV3::sysRenameFolder(const FString& treeId, int treeVersion,
    const FString& updatedName, const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("treeId"), treeId);
    Data->SetNumberField(TEXT("treeVersion"), treeVersion);
    Data->SetStringField(TEXT("updatedName"), updatedName);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysRenameFolder, Data), callback);
}

void US2SGlobalFileV3::sysLookupFolder(const FString& fullFolderPath, const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("fullFolderPath"), fullFolderPath);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysLookupFolder, Data), callback);
}

void US2SGlobalFileV3::sysDeleteFolder(const FString& treeId, const FString& folderPath,
    int treeVersion, bool force, const US2SCallback& callback)
{
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("treeId"), treeId);
    Data->SetStringField(TEXT("folderPath"), folderPath);
    Data->SetNumberField(TEXT("treeVersion"), treeVersion);
    Data->SetBoolField(TEXT("force"), force);
    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysDeleteFolder, Data), callback);
}

// --------------------------------------------------------------------------
// Upload
// --------------------------------------------------------------------------

void US2SGlobalFileV3::uploadGlobalFile(const FString& treeId, const FString& filename,
    bool overwriteIfPresent, const std::vector<uint8_t>& fileData,
    const US2SCallback& callback)
{
    // Step 1 - send SYS_PREPARE_UPLOAD via S2S dispatcher
    TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("treeId"), treeId);
    Data->SetStringField(TEXT("filename"), filename);
    Data->SetBoolField(TEXT("overwriteIfPresent"), overwriteIfPresent);
    Data->SetNumberField(TEXT("fileSize"), static_cast<double>(fileData.size()));

    int32 gen = _generation.load();

    _s2s->request(S2SRequestBuilder::Build(S2SServiceName::GlobalFileV3, S2SServiceOperation::SysPrepareUpload, Data),
        [this, gen, filename, fileData, callback](const FString& result)
        {
            if (gen != _generation) return; // stale after disconnect

            // Parse uploadId from result
            TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(result);
            TSharedPtr<FJsonObject> JsonMsg = MakeShared<FJsonObject>();
            if (FJsonSerializer::Deserialize(Reader, JsonMsg) && JsonMsg->HasField(TEXT("data")))
            {
                TSharedPtr<FJsonObject> JsonData = JsonMsg->GetObjectField(TEXT("data"));
                if (JsonData->HasField(S2SOperationParam::FileDetails))
                {
                    TSharedPtr<FJsonObject> Details = JsonData->GetObjectField(S2SOperationParam::FileDetails);
                    FString UploadId = Details->GetStringField(S2SOperationParam::UploadId);

                    FString FullUploadUrl = _uploadUrl
                        + TEXT("?gameId=") + _s2s->getSessionData().appId
                        + TEXT("&uploadId=") + UploadId;

                    // Use server-supplied uploadUrl if present (mirrors C++ reference behaviour)
                    if (Details->HasField(S2SOperationParam::UploadUrl))
                    {
                        FString ServerUrl = Details->GetStringField(S2SOperationParam::UploadUrl);
                        if (!ServerUrl.IsEmpty())
                        {
                            if (ServerUrl.StartsWith(TEXT("http")))
                            {
                                FullUploadUrl = ServerUrl;
                            }
                            else
                            {
                                // Relative path — prefix with scheme+host from _uploadUrl
                                int32 SchemeEnd = _uploadUrl.Find(TEXT("://"));
                                if (SchemeEnd != INDEX_NONE)
                                {
                                    int32 HostEnd = _uploadUrl.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, SchemeEnd + 3);
                                    if (HostEnd != INDEX_NONE)
                                        FullUploadUrl = _uploadUrl.Left(HostEnd) + ServerUrl;
                                }
                            }
                        }
                    }

                    sendFileUpload(FullUploadUrl, filename, fileData, callback);
                    return;
                }
            }

            // If we get here, the prepare step failed
            if (callback) callback(result);
        });
}

void US2SGlobalFileV3::sendFileUpload(const FString& uploadUrl, const FString& filename,
    const std::vector<uint8_t>& fileData, const US2SCallback& callback)
{
    int32 gen = _generation.load();

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(uploadUrl);
    HttpRequest->SetVerb(TEXT("POST"));

    FString Boundary = FGuid::NewGuid().ToString();
    HttpRequest->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));

    // Build multipart body
    TArray<uint8> Payload;
    {
        FString Header = FString::Printf(TEXT("--%s\r\nContent-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\nContent-Type: application/octet-stream\r\n\r\n"), *Boundary, *filename);
        FTCHARToUTF8 HeaderUtf8(*Header);
        Payload.Append(reinterpret_cast<const uint8*>(HeaderUtf8.Get()), HeaderUtf8.Length());
        Payload.Append(fileData.data(), fileData.size());
        FString Footer = FString::Printf(TEXT("\r\n--%s--\r\n"), *Boundary);
        FTCHARToUTF8 FooterUtf8(*Footer);
        Payload.Append(reinterpret_cast<const uint8*>(FooterUtf8.Get()), FooterUtf8.Length());
    }
    HttpRequest->SetContent(Payload);

    HttpRequest->OnProcessRequestComplete().BindLambda(
        [this, gen, callback](FHttpRequestPtr, FHttpResponsePtr Response, bool bConnectedSuccessfully)
        {
            if (gen != _generation.load()) return;

            FString Result;
            if (bConnectedSuccessfully && Response.IsValid())
            {
                Result = Response->GetContentAsString();
            }
            else
            {
                Result = TEXT("{\"status\":900,\"status_message\":\"Upload HTTP request failed\"}");
            }

            std::lock_guard<std::mutex> Lock(_uploadMutex);
            _completedUploads.push({ callback, Result });
        });

    HttpRequest->ProcessRequest();
}

// --------------------------------------------------------------------------
// Lifecycle
// --------------------------------------------------------------------------

void US2SGlobalFileV3::runCallbacks()
{
    std::queue<UploadCompletion> ToDispatch;
    {
        std::lock_guard<std::mutex> Lock(_uploadMutex);
        std::swap(ToDispatch, _completedUploads);
    }

    while (!ToDispatch.empty())
    {
        auto& Item = ToDispatch.front();
        if (Item.callback) Item.callback(Item.result);
        ToDispatch.pop();
    }
}

void US2SGlobalFileV3::disconnect()
{
    ++_generation;
    std::lock_guard<std::mutex> Lock(_uploadMutex);
    while (!_completedUploads.empty()) _completedUploads.pop();
}

void US2SGlobalFileV3::log(const FString& message)
{
    UE_LOG(LogBrainCloudS2S, Log, TEXT("[S2SGlobalFileV3] %s"), *message);
}

// --------------------------------------------------------------------------
// Blueprint wrapper helper
// --------------------------------------------------------------------------

US2SCallback US2SGlobalFileV3::WrapBPDelegate(const FS2SResponseDelegate& Delegate)
{
    // Copy the delegate so it survives the lambda capture
    FS2SResponseDelegate DelegateCopy = Delegate;
    return [DelegateCopy](const FString& JsonResponse)
    {
        // Determine success by parsing the status field
        bool bSuccess = false;
        TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(JsonResponse);
        TSharedPtr<FJsonObject> JsonObj;
        if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj->HasField(TEXT("status")))
        {
            bSuccess = (JsonObj->GetIntegerField(TEXT("status")) == 200);
        }
        DelegateCopy.ExecuteIfBound(bSuccess, JsonResponse);
    };
}

// --------------------------------------------------------------------------
// Blueprint wrappers
// --------------------------------------------------------------------------

void US2SGlobalFileV3::S2S_SysGetFileInfo(const FString& FileId, const FS2SResponseDelegate& Callback)
{
    sysGetFileInfo(FileId, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysGetFileInfoSimple(const FString& FolderPath, const FString& Filename, const FS2SResponseDelegate& Callback)
{
    sysGetFileInfoSimple(FolderPath, Filename, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysCheckFilenameExists(const FString& FolderPath, const FString& Filename, const FS2SResponseDelegate& Callback)
{
    sysCheckFilenameExists(FolderPath, Filename, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysCheckFullpathFilenameExists(const FString& FullpathFilename, const FS2SResponseDelegate& Callback)
{
    sysCheckFullpathFilenameExists(FullpathFilename, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysGetGlobalCDNUrl(const FString& FileId, const FS2SResponseDelegate& Callback)
{
    sysGetGlobalCDNUrl(FileId, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysGetGlobalFileList(const FString& FolderPath, bool bRecurse, const FS2SResponseDelegate& Callback)
{
    sysGetGlobalFileList(FolderPath, bRecurse, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysMoveToGlobalFile(const FString& UserProfileId, const FString& UserCloudPath,
    const FString& UserCloudFilename, const FString& GlobalTreeId,
    const FString& GlobalFilename, bool bOverwriteIfPresent,
    const FS2SResponseDelegate& Callback)
{
    sysMoveToGlobalFile(UserProfileId, UserCloudPath, UserCloudFilename, GlobalTreeId, GlobalFilename, bOverwriteIfPresent, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysCopyGlobalFile(const FString& FileId, int32 Version,
    const FString& NewTreeId, int32 TreeVersion,
    const FString& NewFilename, bool bOverwriteIfPresent,
    const FS2SResponseDelegate& Callback)
{
    sysCopyGlobalFile(FileId, Version, NewTreeId, TreeVersion, NewFilename, bOverwriteIfPresent, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysMoveGlobalFile(const FString& FileId, int32 Version,
    const FString& NewTreeId, int32 TreeVersion,
    const FString& NewFilename, bool bOverwriteIfPresent,
    const FS2SResponseDelegate& Callback)
{
    sysMoveGlobalFile(FileId, Version, NewTreeId, TreeVersion, NewFilename, bOverwriteIfPresent, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysDeleteGlobalFile(const FString& FileId, int32 Version,
    const FString& Filename, const FS2SResponseDelegate& Callback)
{
    sysDeleteGlobalFile(FileId, Version, Filename, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysDeleteGlobalFiles(const FString& TreeId, const FString& FolderPath,
    int32 TreeVersion, bool bRecurse, const FS2SResponseDelegate& Callback)
{
    sysDeleteGlobalFiles(TreeId, FolderPath, TreeVersion, bRecurse, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysCreateFolder(const FString& FolderPath, int32 TreeVersion,
    const FString& Name, const FString& Desc,
    bool bCreateInterimDirectories, const FS2SResponseDelegate& Callback)
{
    sysCreateFolder(FolderPath, TreeVersion, Name, Desc, bCreateInterimDirectories, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysMoveFolder(const FString& TreeId, int32 TreeVersion,
    const FString& NewFolderPath, const FString& UpdatedName,
    bool bCreateInterimDirectories, const FS2SResponseDelegate& Callback)
{
    sysMoveFolder(TreeId, TreeVersion, NewFolderPath, UpdatedName, bCreateInterimDirectories, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysRenameFolder(const FString& TreeId, int32 TreeVersion,
    const FString& UpdatedName, const FS2SResponseDelegate& Callback)
{
    sysRenameFolder(TreeId, TreeVersion, UpdatedName, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysLookupFolder(const FString& FullFolderPath, const FS2SResponseDelegate& Callback)
{
    sysLookupFolder(FullFolderPath, WrapBPDelegate(Callback));
}

void US2SGlobalFileV3::S2S_SysDeleteFolder(const FString& TreeId, const FString& FolderPath,
    int32 TreeVersion, bool bForce, const FS2SResponseDelegate& Callback)
{
    sysDeleteFolder(TreeId, FolderPath, TreeVersion, bForce, WrapBPDelegate(Callback));
}
