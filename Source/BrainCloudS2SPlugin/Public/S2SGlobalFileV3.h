// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BrainCloudS2STypes.h"

#include <functional>
#include <mutex>
#include <queue>

#include "S2SGlobalFileV3.generated.h"

using US2SCallback = std::function<void(const FString&)>;

class UBrainCloudS2S;

/**
* S2S service for brainCloud Global File V3 operations.
*
* Access via UBrainCloudS2S::GetGlobalFileV3() after creating a context.
*
* File upload is a two-step process:
*   1. SYS_PREPARE_UPLOAD is sent via the S2S dispatcher and returns an uploadId.
*   2. The file bytes are POSTed as multipart/form-data to the upload endpoint.
*
* The upload callback is dispatched on the next call to UBrainCloudS2S::runCallbacks()
* after the upload HTTP request completes.
*/

UCLASS(BlueprintType)
class BRAINCLOUDS2SPLUGIN_API US2SGlobalFileV3 : public UObject
{
	GENERATED_BODY()

public:
	US2SGlobalFileV3();

	/** Called internally to wire this service to its owning S2S context. */
	void SetS2SContext(UBrainCloudS2S* context);

	/**
	* Derives the upload URL from the S2S dispatcher URL. Called by S2SContext automatically.
	*/
	void init(const FString& serverUrl);

	// -----------------------------------------------------------------------
	// File Info / Query
	// -----------------------------------------------------------------------

	/** Returns metadata for a global file identified by its fileId. */
	void sysGetFileInfo(const FString& fileId, const US2SCallback& callback);

	/** Returns metadata for a global file identified by folder path and filename. */
	void sysGetFileInfoSimple(const FString& folderPath, const FString& filename,
		const US2SCallback& callback);

	/** Returns true if a file with the given name exists in the specified folder. */
	void sysCheckFilenameExists(const FString& folderPath, const FString& filename,
		const US2SCallback& callback);

	/** Returns true if a file exists at the given full path (e.g. "folder/sub/file.txt"). */
	void sysCheckFullpathFilenameExists(const FString& fullpathFilename,
		const US2SCallback& callback);

	/** Returns the CDN URL for the global file identified by fileId. */
	void sysGetGlobalCDNUrl(const FString& fileId, const US2SCallback& callback);

	/**
	* Lists all global files under the given folder path.
	* Pass folderPath="" and recurse=true to list the entire tree.
	*/
	void sysGetGlobalFileList(const FString& folderPath, bool recurse,
		const US2SCallback& callback);

	// -----------------------------------------------------------------------
	// File Management
	// -----------------------------------------------------------------------

	/** Moves a file from personal cloud storage into the global file system. */
	void sysMoveToGlobalFile(const FString& userProfileId, const FString& userCloudPath,
		const FString& userCloudFilename, const FString& globalTreeId,
		const FString& globalFilename, bool overwriteIfPresent,
		const US2SCallback& callback);

	/**
	* Copies a global file to another folder, optionally with a new name.
	* Pass version=-1 to copy the latest version. Pass treeVersion=-1 to skip the version check.
	*/
	void sysCopyGlobalFile(const FString& fileId, int version,
		const FString& newTreeId, int treeVersion,
		const FString& newFilename, bool overwriteIfPresent,
		const US2SCallback& callback);

	/**
	* Moves a global file to another folder, optionally with a new name.
	* Pass version=-1 to move the latest version. Pass treeVersion=-1 to skip the version check.
	*/
	void sysMoveGlobalFile(const FString& fileId, int version,
		const FString& newTreeId, int treeVersion,
		const FString& newFilename, bool overwriteIfPresent,
		const US2SCallback& callback);

	/** Deletes a single global file. Pass version=-1 to delete without a version check. */
	void sysDeleteGlobalFile(const FString& fileId, int version,
		const FString& filename, const US2SCallback& callback);

	/**
	* Deletes all global files in the specified folder.
	* Pass treeVersion=-1 to skip the version check. Set recurse=true to include sub-folders.
	*/
	void sysDeleteGlobalFiles(const FString& treeId, const FString& folderPath,
		int treeVersion, bool recurse, const US2SCallback& callback);

	// -----------------------------------------------------------------------
	// Folder Management
	// -----------------------------------------------------------------------

	/**
	* Creates a new folder at the given path.
	* Pass treeVersion=-1 to skip the version check.
	* Set createInterimDirectories=true to auto-create any missing parent folders.
	*/
	void sysCreateFolder(const FString& folderPath, int treeVersion,
		const FString& name, const FString& desc,
		bool createInterimDirectories, const US2SCallback& callback);

	/**
	 * Moves a folder to a new path, optionally renaming it.
	 * Pass treeVersion=-1 to skip the version check.
	 */
	void sysMoveFolder(const FString& treeId, int treeVersion,
		const FString& newFolderPath, const FString& updatedName,
		bool createInterimDirectories, const US2SCallback& callback);

	/**
	 * Renames a folder in place.
	 * Pass treeVersion=-1 to skip the version check.
	 */
	void sysRenameFolder(const FString& treeId, int treeVersion,
		const FString& updatedName, const US2SCallback& callback);

	/** Resolves the treeId for a folder given its full path. */
	void sysLookupFolder(const FString& fullFolderPath, const US2SCallback& callback);

	/**
	 * Deletes a folder. Set force=true to also delete any files and sub-folders inside it.
	 * Pass treeVersion=-1 to skip the version check.
	 */
	void sysDeleteFolder(const FString& treeId, const FString& folderPath,
		int treeVersion, bool force, const US2SCallback& callback);

	// -----------------------------------------------------------------------
	// Upload
	// -----------------------------------------------------------------------

	/**
	 * Uploads a file to the brainCloud Global File V3 system via S2S.
	 *
	 * NOTE: Not exposed to Blueprints because fileData is a std::vector<uint8_t>,
	 * which is not a Blueprint-compatible type.
	 *
	 * @param treeId              Folder tree ID (use "_root_" for root)
	 * @param filename            Name of the file as it will appear in brainCloud
	 * @param overwriteIfPresent  Replaces any existing file with the same name when true
	 * @param fileData            File content as bytes
	 * @param callback            Invoked with the upload result JSON string
	 */
	void uploadGlobalFile(const FString& treeId, const FString& filename,
		bool overwriteIfPresent, const std::vector<uint8_t>& fileData,
		const US2SCallback& callback);

	// -----------------------------------------------------------------------
	// Lifecycle (called internally by S2SContext)
	// -----------------------------------------------------------------------

	/** Dispatches completed upload callbacks. Called by S2SContext::runCallbacks(). */
	void runCallbacks();

	/** Cancels pending upload callbacks. Called by S2SContext::disconnect(). */
	void disconnect();

	// -----------------------------------------------------------------------
	// Blueprint wrappers (S2S_ prefix)
	// -----------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Get File Info"))
	void S2S_SysGetFileInfo(const FString& FileId, const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Get File Info Simple"))
	void S2S_SysGetFileInfoSimple(const FString& FolderPath, const FString& Filename, const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Check Filename Exists"))
	void S2S_SysCheckFilenameExists(const FString& FolderPath, const FString& Filename, const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Check Fullpath Filename Exists"))
	void S2S_SysCheckFullpathFilenameExists(const FString& FullpathFilename, const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Get Global CDN Url"))
	void S2S_SysGetGlobalCDNUrl(const FString& FileId, const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Get Global File List"))
	void S2S_SysGetGlobalFileList(const FString& FolderPath, bool bRecurse, const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Move To Global File"))
	void S2S_SysMoveToGlobalFile(const FString& UserProfileId, const FString& UserCloudPath,
		const FString& UserCloudFilename, const FString& GlobalTreeId,
		const FString& GlobalFilename, bool bOverwriteIfPresent,
		const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Copy Global File"))
	void S2S_SysCopyGlobalFile(const FString& FileId, int32 Version,
		const FString& NewTreeId, int32 TreeVersion,
		const FString& NewFilename, bool bOverwriteIfPresent,
		const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Move Global File"))
	void S2S_SysMoveGlobalFile(const FString& FileId, int32 Version,
		const FString& NewTreeId, int32 TreeVersion,
		const FString& NewFilename, bool bOverwriteIfPresent,
		const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Delete Global File"))
	void S2S_SysDeleteGlobalFile(const FString& FileId, int32 Version,
		const FString& Filename, const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Delete Global Files"))
	void S2S_SysDeleteGlobalFiles(const FString& TreeId, const FString& FolderPath,
		int32 TreeVersion, bool bRecurse, const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Create Folder"))
	void S2S_SysCreateFolder(const FString& FolderPath, int32 TreeVersion,
		const FString& Name, const FString& Desc,
		bool bCreateInterimDirectories, const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Move Folder"))
	void S2S_SysMoveFolder(const FString& TreeId, int32 TreeVersion,
		const FString& NewFolderPath, const FString& UpdatedName,
		bool bCreateInterimDirectories, const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Rename Folder"))
	void S2S_SysRenameFolder(const FString& TreeId, int32 TreeVersion,
		const FString& UpdatedName, const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Lookup Folder"))
	void S2S_SysLookupFolder(const FString& FullFolderPath, const FS2SResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|GlobalFileV3", meta = (DisplayName = "Sys Delete Folder"))
	void S2S_SysDeleteFolder(const FString& TreeId, const FString& FolderPath,
		int32 TreeVersion, bool bForce, const FS2SResponseDelegate& Callback);

private:
	UPROPERTY()
	UBrainCloudS2S* _s2s = nullptr;

	FString _uploadUrl;
	std::atomic<int> _generation;

	struct UploadCompletion
	{
		US2SCallback callback;
		FString result;
	};

	std::mutex _uploadMutex;
	std::queue<UploadCompletion> _completedUploads;

	void sendFileUpload(const FString& uploadUrl, const FString& filename,
		const std::vector<uint8_t>& fileData, const US2SCallback& callback);

	void log(const FString& message);

	/** Helper: wraps an FS2SResponseDelegate into a US2SCallback. */
	static US2SCallback WrapBPDelegate(const FS2SResponseDelegate& Delegate);
};
