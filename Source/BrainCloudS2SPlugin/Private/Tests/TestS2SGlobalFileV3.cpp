// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/S2STestFixture.h"
#include "BrainCloudS2S.h"
#include "S2SGlobalFileV3.h"
#include "Misc/Base64.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

using namespace S2STestFixture;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace
{
    static UBrainCloudS2S* MakeContext(FAutomationTestBase& Test)
    {
        if (!CredentialsLoaded())
        {
            Test.AddError(TEXT("Missing credentials. Set BC_APP_ID, BC_SERVER_NAME, BC_SERVER_SECRET, BC_S2S_URL."));
            return nullptr;
        }

        UBrainCloudS2S* Ctx = UBrainCloudS2S::CreateS2SContext(
            AppId(), ServerName(), ServerSecret(), ServerUrl(), /*bAutoAuth=*/false);
        Ctx->AddToRoot();
        Ctx->setLogEnabled(true);

        FString AuthResult = RunCall(Ctx, [&](US2SCallback CB) { Ctx->authenticate(CB); });
        if (GetStatus(AuthResult) != 200)
        {
            Test.AddError(FString::Printf(TEXT("Authentication failed: %s"), *AuthResult));
            Ctx->RemoveFromRoot();
            return nullptr;
        }
        return Ctx;
    }

    static FString Base64Encode(const FString& Text)
    {
        FTCHARToUTF8 UTF8(*Text);
        return FBase64::Encode(
            reinterpret_cast<const uint8*>(UTF8.Get()),
            static_cast<uint32>(UTF8.Length()));
    }

    // Extract an int32 from response.data.<FieldName>. Returns DefaultValue on failure.
    static int32 GetDataIntField(const FString& Json, const FString& FieldName, int32 DefaultValue = 0)
    {
        TSharedPtr<FJsonObject> Obj;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
        if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid()) return DefaultValue;
        const TSharedPtr<FJsonObject>* Data;
        if (!Obj->TryGetObjectField(TEXT("data"), Data)) return DefaultValue;
        double Val = DefaultValue;
        (*Data)->TryGetNumberField(FieldName, Val);
        return (int32)Val;
    }
}

// ---------------------------------------------------------------------------
// Test 1: SysGetGlobalFileList — basic root-level query
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FS2SGFv3GetFileListTest,
    "BrainCloudS2S.GlobalFileV3.SysGetGlobalFileList",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ProductFilter)

bool FS2SGFv3GetFileListTest::RunTest(const FString& /*Parameters*/)
{
    UBrainCloudS2S* Ctx = MakeContext(*this);
    if (!Ctx) return false;
    ON_SCOPE_EXIT { Ctx->disconnect(); Ctx->RemoveFromRoot(); };

    FString Result = RunCall(Ctx, [&](US2SCallback CB)
    {
        Ctx->GetGlobalFileV3()->sysGetGlobalFileList(TEXT(""), /*bRecurse=*/false, CB);
    });
    TestEqual(TEXT("SysGetGlobalFileList status"), GetStatus(Result), 200);

    return !HasAnyErrors();
}

// ---------------------------------------------------------------------------
// Test 2: Full create → upload → query → mutate → cleanup sequence
//
//   01 SysCreateFolder                — create s2s_ue_test_folder, capture treeId
//   02 SysLookupFolder                — verify folder exists
//   03 SysUploadStream (new)          — upload file with overwriteIfPresent=false, capture fileId/version
//   04 SysUploadStream (overwrite)    — re-upload same filename with overwrite=true
//   05 SysGetFileInfo                 — verify file by fileId
//   06 SysGetFileInfoSimple           — verify file by folder path + name
//   07 SysCheckFilenameExists         — confirm filename found in folder
//   08 SysCheckFullpathFilenameExists — confirm file exists at full path
//   09 SysGetGlobalCDNUrl             — get CDN URL for the file
//   10 SysCopyGlobalFile              — copy file within the same folder
//   11 SysMoveGlobalFile              — move original file to a new name
//   12 SysRenameFolder                — rename the test folder
//   13 SysDeleteGlobalFiles           — delete all files (cleanup)
//   14 SysDeleteFolder                — delete folder (cleanup)
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FS2SGFv3FullSequenceTest,
    "BrainCloudS2S.GlobalFileV3.FullSequence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ProductFilter)

bool FS2SGFv3FullSequenceTest::RunTest(const FString& /*Parameters*/)
{
    UBrainCloudS2S* Ctx = MakeContext(*this);
    if (!Ctx) return false;
    ON_SCOPE_EXIT { Ctx->disconnect(); Ctx->RemoveFromRoot(); };

    US2SGlobalFileV3* GFV3 = Ctx->GetGlobalFileV3();
    const FString FolderName        = TEXT("s2s_ue_test_folder");
    const FString RenamedFolderName = TEXT("s2s_ue_test_folder_renamed");
    const FString FileName          = TEXT("s2s_ue_test_file.txt");
    FString FolderTreeId;
    FString FileId;
    int32   FileVersion = 1;

    // 01: SysCreateFolder
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysCreateFolder(TEXT(""), /*treeVersion=*/-1, FolderName,
                TEXT("Unreal S2S integration test folder"), /*createInterim=*/true, CB);
        });
        if (!TestEqual(TEXT("01 SysCreateFolder"), GetStatus(Result), 200)) return false;

        FolderTreeId = GetDataStringField(Result, TEXT("createdTreeId"));
        if (!TestTrue(TEXT("01 createdTreeId not empty"), !FolderTreeId.IsEmpty())) return false;
    }

    // 02: SysLookupFolder
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysLookupFolder(FolderName, CB);
        });
        TestEqual(TEXT("02 SysLookupFolder"), GetStatus(Result), 200);
    }

    // 03: SysUploadStream — upload a new file, capture fileId + version
    {
        const FString FileData = Base64Encode(
            TEXT("Hello from brainCloud Unreal S2S upload stream test!"));

        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysUploadStream(FolderTreeId, FileName,
                /*overwriteIfPresent=*/false, FileData, CB);
        });
        if (!TestEqual(TEXT("03 SysUploadStream (new file)"), GetStatus(Result), 200)) return false;

        TSharedPtr<FJsonObject> Root;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Result);
        if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
        {
            const TSharedPtr<FJsonObject>* DataObj;
            const TSharedPtr<FJsonObject>* FileDetailsObj;
            if (Root->TryGetObjectField(TEXT("data"), DataObj) &&
                (*DataObj)->TryGetObjectField(TEXT("fileDetails"), FileDetailsObj))
            {
                (*FileDetailsObj)->TryGetStringField(TEXT("fileId"), FileId);
                double VersionD = 1.0;
                (*FileDetailsObj)->TryGetNumberField(TEXT("version"), VersionD);
                FileVersion = (int32)VersionD;
            }
        }
        if (!TestTrue(TEXT("03 fileId not empty"), !FileId.IsEmpty())) return false;
    }

    // 04: SysUploadStream — overwrite with updated content
    {
        const FString UpdatedData = Base64Encode(
            TEXT("Updated content from SysUploadStream overwrite test."));

        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysUploadStream(FolderTreeId, FileName,
                /*overwriteIfPresent=*/true, UpdatedData, CB);
        });
        if (!TestEqual(TEXT("04 SysUploadStream (overwrite)"), GetStatus(Result), 200)) return false;

        // Overwrite creates a brand-new fileId/version — update so later steps use the live values.
        TSharedPtr<FJsonObject> Root04;
        TSharedRef<TJsonReader<>> Reader04 = TJsonReaderFactory<>::Create(Result);
        if (FJsonSerializer::Deserialize(Reader04, Root04) && Root04.IsValid())
        {
            const TSharedPtr<FJsonObject>* DataObj04;
            const TSharedPtr<FJsonObject>* FileDetailsObj04;
            if (Root04->TryGetObjectField(TEXT("data"), DataObj04) &&
                (*DataObj04)->TryGetObjectField(TEXT("fileDetails"), FileDetailsObj04))
            {
                FString NewFileId;
                (*FileDetailsObj04)->TryGetStringField(TEXT("fileId"), NewFileId);
                if (!NewFileId.IsEmpty())
                {
                    FileId = NewFileId;
                    double VersionD = (double)FileVersion;
                    (*FileDetailsObj04)->TryGetNumberField(TEXT("version"), VersionD);
                    FileVersion = (int32)VersionD;
                }
            }
        }
    }

    // 05: SysGetFileInfo
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysGetFileInfo(FileId, CB);
        });
        TestEqual(TEXT("05 SysGetFileInfo"), GetStatus(Result), 200);
    }

    // 06: SysGetFileInfoSimple
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysGetFileInfoSimple(FolderName, FileName, CB);
        });
        TestEqual(TEXT("06 SysGetFileInfoSimple"), GetStatus(Result), 200);
    }

    // 07: SysCheckFilenameExists
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysCheckFilenameExists(FolderName, FileName, CB);
        });
        TestEqual(TEXT("07 SysCheckFilenameExists"), GetStatus(Result), 200);
    }

    // 08: SysCheckFullpathFilenameExists
    {
        const FString FullPath = FolderName + TEXT("/") + FileName;
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysCheckFullpathFilenameExists(FullPath, CB);
        });
        TestEqual(TEXT("08 SysCheckFullpathFilenameExists"), GetStatus(Result), 200);
    }

    // 09: SysGetGlobalCDNUrl
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysGetGlobalCDNUrl(FileId, CB);
        });
        TestEqual(TEXT("09 SysGetGlobalCDNUrl"), GetStatus(Result), 200);
    }

    // 10: SysCopyGlobalFile — copy to same folder under a different name
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysCopyGlobalFile(FileId, FileVersion, FolderTreeId, /*treeVersion=*/-1,
                TEXT("s2s_ue_file_copy.txt"), /*overwrite=*/true, CB);
        });
        TestEqual(TEXT("10 SysCopyGlobalFile"), GetStatus(Result), 200);
    }

    // 11: SysMoveGlobalFile — move the original file to a different name
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysMoveGlobalFile(FileId, FileVersion, FolderTreeId, /*treeVersion=*/-1,
                TEXT("s2s_ue_file_moved.txt"), /*overwrite=*/true, CB);
        });
        TestEqual(TEXT("11 SysMoveGlobalFile"), GetStatus(Result), 200);
    }

    // 12: SysRenameFolder
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysRenameFolder(FolderTreeId, /*treeVersion=*/-1, RenamedFolderName, CB);
        });
        TestEqual(TEXT("12 SysRenameFolder"), GetStatus(Result), 200);
    }

    // 13: SysDeleteGlobalFiles — remove all files in the renamed folder
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysDeleteGlobalFiles(FolderTreeId, RenamedFolderName, /*treeVersion=*/-1,
                /*recurse=*/true, CB);
        });
        TestEqual(TEXT("13 SysDeleteGlobalFiles (cleanup)"), GetStatus(Result), 200);
    }

    // 14: SysDeleteFolder — remove the test folder itself
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysDeleteFolder(FolderTreeId, RenamedFolderName, /*treeVersion=*/-1,
                /*force=*/true, CB);
        });
        TestEqual(TEXT("14 SysDeleteFolder (cleanup)"), GetStatus(Result), 200);
    }

    return !HasAnyErrors();
}

// ---------------------------------------------------------------------------
// Test 3: SysUploadStream — overwriteIfPresent=false rejects a duplicate
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FS2SGFv3UploadStreamNoOverwriteTest,
    "BrainCloudS2S.GlobalFileV3.SysUploadStream_NoOverwrite",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ProductFilter)

bool FS2SGFv3UploadStreamNoOverwriteTest::RunTest(const FString& /*Parameters*/)
{
    UBrainCloudS2S* Ctx = MakeContext(*this);
    if (!Ctx) return false;
    ON_SCOPE_EXIT { Ctx->disconnect(); Ctx->RemoveFromRoot(); };

    US2SGlobalFileV3* GFV3 = Ctx->GetGlobalFileV3();
    const FString FolderName = TEXT("s2s_ue_nooverwrite_folder");
    const FString FileName   = TEXT("s2s_ue_nooverwrite.txt");
    FString FolderTreeId;

    // Setup: create a temporary folder
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysCreateFolder(TEXT(""), -1, FolderName,
                TEXT("NoOverwrite test folder"), true, CB);
        });
        if (!TestEqual(TEXT("Setup SysCreateFolder"), GetStatus(Result), 200)) return false;
        FolderTreeId = GetDataStringField(Result, TEXT("createdTreeId"));
        if (!TestTrue(TEXT("Setup createdTreeId not empty"), !FolderTreeId.IsEmpty())) return false;
    }

    const FString FileData = Base64Encode(TEXT("original content"));

    // First upload — new file, should succeed
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysUploadStream(FolderTreeId, FileName,
                /*overwriteIfPresent=*/false, FileData, CB);
        });
        TestEqual(TEXT("First upload (new file)"), GetStatus(Result), 200);
    }

    // Second upload of same filename with overwriteIfPresent=false — server must reject it
    {
        FString Result = RunCall(Ctx, [&](US2SCallback CB)
        {
            GFV3->sysUploadStream(FolderTreeId, FileName,
                /*overwriteIfPresent=*/false, FileData, CB);
        });
        TestNotEqual(TEXT("Duplicate upload (no-overwrite) rejected"), GetStatus(Result), 200);
    }

    // Cleanup — best-effort; errors here don't fail the test
    RunCall(Ctx, [&](US2SCallback CB)
    {
        GFV3->sysDeleteGlobalFiles(FolderTreeId, FolderName, -1, true, CB);
    });
    RunCall(Ctx, [&](US2SCallback CB)
    {
        GFV3->sysDeleteFolder(FolderTreeId, FolderName, -1, true, CB);
    });

    return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
