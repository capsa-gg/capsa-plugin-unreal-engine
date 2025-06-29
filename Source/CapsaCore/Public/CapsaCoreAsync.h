// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "CapsaCore.h"
#include "CapsaLogOperations.h"

typedef TFunction<void(const FString&)> FAsyncStringFromBufferCallback;
typedef TFunction<void(const TArray<uint8>&)> FAsyncBinaryFromBufferCallback;


/// Base Capsa Async Task. Stores the Buffer and Callback function. Also contains base helper methods like those to construct a single Log String from the Buffer.
template<typename CallbackType>
class FCapsaAsyncTask : public FNonAbandonableTask
{
public:
	friend class FAutoDeleteAsyncTask<FCapsaAsyncTask>;

	FCapsaAsyncTask(TArray<FBufferedLine> InBuffer, CallbackType InCallbackFunction) :
		Buffer(MoveTemp(InBuffer)),
		CallbackFunction(InCallbackFunction),
		LogExtension(CapsaLogOperations::DefaultUncompressedLogExtension),
		CompressedExtension(CapsaLogOperations::DefaultCompressedLogExtension)
	{
	}

	FString MakeLogString() const
	{
		return CapsaLogOperations::MakeLogString(Buffer);
	}

	bool MakeCompressedLogBinary(FString& UncompressedLog, TArray<uint8>& BinaryData) const
	{
		UE_LOG(LogCapsaCore, VeryVerbose, TEXT("FCapsaAsyncTask::MakeCompressedLogBinary | Start compression"))

		// Get log string, uncompressed
		UncompressedLog = MakeLogString();
		UE_LOG(LogCapsaCore, VeryVerbose, TEXT("FCapsaAsyncTask::MakeCompressedLogBinary | Uncompressed log length: %d"), UncompressedLog.Len());

		// Convert log string to uint8*
		return CapsaLogOperations::MakeCompressedLogBinary(UncompressedLog, BinaryData);
	}

	bool SaveStringToFile(const FString& LogToSave, const FString& FileName) const
	{
		return CapsaLogOperations::SaveStringToFile(LogToSave, FileName, LogExtension);
	}

	bool SaveBinaryToFile(const TArray<uint8>& BinaryData, const FString& FileName) const
	{
		return CapsaLogOperations::SaveBinaryToFile(BinaryData, FileName, CompressedExtension);
	}

	void DoWork() const
	{
		// Default does nothing.
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FCapsaAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

protected:
	TArray<FBufferedLine> Buffer;
	CallbackType CallbackFunction;
	const FString LogExtension;
	const FString CompressedExtension;
};

/// Async task to create a FString that we can send over HTTP from a FBufferredLine TArray and then save this Raw String to File.
class FSaveStringFromBufferTask : public FCapsaAsyncTask<FAsyncStringFromBufferCallback>
{
public:
	friend class FAutoDeleteAsyncTask<FSaveStringFromBufferTask>;

	FSaveStringFromBufferTask(FString InLogID, bool bInWriteToDisk, TArray<FBufferedLine> InBuffer, FAsyncStringFromBufferCallback InCallbackFunction) :
		FCapsaAsyncTask<FAsyncStringFromBufferCallback>(MoveTemp(InBuffer), InCallbackFunction),
		LogID(InLogID),
		bWriteToDiskPlain(bInWriteToDisk)
	{
	}

	void DoWork() const
	{
		FString Log = MakeLogString();
		if (bWriteToDiskPlain)
		{
			if (!SaveStringToFile(Log, LogID))
			{
				UE_LOG(LogCapsaCore, Warning, TEXT( "Failed to write plain text file to disk" ))
			}
		}
		CallbackFunction(Log);
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FSaveStringFromBufferTask, STATGROUP_ThreadPoolAsyncTasks);
	}

protected:
	FString LogID;
	bool bWriteToDiskPlain;
};

/// Async task to create a Binary Array that we can send over HTTP from a FBufferredLine TArray and then save this compressed Binary Array to File.
class FSaveCompressedStringFromBufferTask : public FCapsaAsyncTask<FAsyncBinaryFromBufferCallback>
{
public:
	friend class FAutoDeleteAsyncTask<FSaveCompressedStringFromBufferTask>;

	FSaveCompressedStringFromBufferTask(FString InLogID, bool bInWriteToDiskPlain, bool bInWriteToDiskCompressed, TArray<FBufferedLine> InBuffer,
		FAsyncBinaryFromBufferCallback InCallbackFunction) :
		FCapsaAsyncTask(MoveTemp(InBuffer), InCallbackFunction),
		LogID(InLogID),
		bWriteToDiskPlain(bInWriteToDiskPlain),
		bWriteToDiskCompressed(bInWriteToDiskCompressed)
	{
	}

	void DoWork() const
	{
		FString Log;
		TArray<uint8> CompressedLog;

		// Compress data
		if (!MakeCompressedLogBinary(Log, CompressedLog))
		{
			UE_LOG(LogCapsaCore, Warning, TEXT( "FSaveCompressedStringFromBufferTask::DoWork | Failed to compress log binary" ))
		}
		else
		{
			UE_LOG(LogCapsaCore, VeryVerbose, TEXT( "FSaveCompressedStringFromBufferTask::DoWork | Compressed log binary, length: %d" ), CompressedLog.Num())

			// Save compressed file to disk
			if (bWriteToDiskCompressed)
			{
				if (!SaveBinaryToFile(CompressedLog, LogID))
				{
					UE_LOG(LogCapsaCore, Warning, TEXT( "FSaveCompressedStringFromBufferTask::DoWork | Failed to write compressed file to disk" ))
				}
			}
		}

		// Save plain text to disk
		if (bWriteToDiskPlain)
		{
			if (!SaveStringToFile(Log, LogID))
			{
				UE_LOG(LogCapsaCore, Warning, TEXT( "FSaveCompressedStringFromBufferTask::DoWork | Failed to write plain text file to disk" ))
			}
		}

		CallbackFunction(CompressedLog);
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FSaveCompressedStringFromBufferTask, STATGROUP_ThreadPoolAsyncTasks);
	}

protected:
	FString LogID;
	bool bWriteToDiskPlain;
	bool bWriteToDiskCompressed;
};
