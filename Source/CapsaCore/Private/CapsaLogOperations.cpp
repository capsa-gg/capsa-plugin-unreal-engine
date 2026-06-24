// Copyright Companion Group, Ltd. Made available under the MIT license

#include "CapsaLogOperations.h"

#include "CapsaCore.h"

#include "CoreMinimal.h"
#include "FunctionLibrary/CapsaCoreFunctionLibrary.h"

namespace CapsaLogOperations
{
FString MakeLogString(const TArray<FBufferedLine>& Buffer)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(MakeLogString);

	FString Log;
	for (const FBufferedLine& Line : Buffer)
	{
		// Construct the Time from the Seconds when the Line was added
		FDateTime Time = FDateTime::FromUnixTimestampDecimal(Line.Time);
		FWideStringBuilderBase TimeStamp;
		// format: yyyy.mm.dd-hh.mm.ss:mil
		Log.Append(FString::Printf(TEXT("[%s]"), *Time.ToString(TEXT("%Y.%m.%d-%H.%M.%S.%s"))));
		Log.Append(FString::Printf(TEXT("[%s]"), *UCapsaCoreFunctionLibrary::GetLogVerbosityString(Line.Verbosity)));
		Log.Append(FString::Printf(TEXT("[%s]: "), *Line.Category.Resolve().ToString()));
		Log.Append(Line.Data.Get());
		Log.Append(LINE_TERMINATOR_ANSI); // Use lf ending on all platforms
	}

	return Log;
}

bool MakeCompressedLogBinary(const FString& UncompressedLog, TArray<uint8>& BinaryData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(MakeCompressedLogBinary);

	TArray<uint8> UncompressedLogBytes;
	uint64 Utf8Length = FPlatformString::ConvertedLength<UTF8CHAR>(*UncompressedLog, UncompressedLog.Len());
	UncompressedLogBytes.SetNumUninitialized(Utf8Length);
	FPlatformString::Convert((UTF8CHAR*)UncompressedLogBytes.GetData(), UncompressedLogBytes.Num(), *UncompressedLog, UncompressedLog.Len());

	// Reserve memory for compressed data, setting 4/3 in case compressed turns out longer
	BinaryData.SetNumUninitialized(UncompressedLog.Len() * 4 / 3);
	int32 CompressedSize = BinaryData.Num();
	UE_LOG(LogCapsaCore, VeryVerbose, TEXT("FCapsaAsyncTask::FCapsaAsyncTask | Utf8Length: %llu"), Utf8Length)

	// Compress data
	const bool bSuccess = FCompression::CompressMemory(
		NAME_Zlib,
		BinaryData.GetData(),
		CompressedSize,
		UncompressedLogBytes.GetData(),
		UncompressedLogBytes.Num()
		);

	UE_LOG(LogCapsaCore, Verbose, TEXT( "FCapsaAsyncTask::MakeCompressedLogBinary | Success: %d, compressed size: %d" ), bSuccess, CompressedSize);

	return bSuccess;
}

bool SaveStringToFile(const FString& LogToSave, const FString& FileName, const FString& FileExtension)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(SaveStringToFile);

	FString FilePath = FPaths::ProjectLogDir() + FileName + FileExtension;

	UE_LOG(LogCapsaCore, Verbose, TEXT( "FCapsaAsyncTask::SaveStringToFile | Attempting to write/append to: %s" ), *FilePath);

	return FFileHelper::SaveStringToFile(LogToSave, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(),
		EFileWrite::FILEWRITE_Append);
}

bool SaveBinaryToFile(TArray<uint8> BinaryData, const FString& FileName, const FString& FileExtension)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(SaveBinaryToFile);

	FString CapsaCompressedDirectory = TEXT("CapsaCompressedChunks/") + FileName + TEXT("/");
	FString FilePath = FPaths::ProjectLogDir() + CapsaCompressedDirectory + FDateTime::Now().ToString(TEXT("%Y-%m-%dT%H.%M.%S.%s")) + FileExtension;

	UE_LOG(LogCapsaCore, Verbose, TEXT( "FCapsaAsyncTask::SaveBinaryToFile | Attempting to write to: %s" ), *FilePath);

	return FFileHelper::SaveArrayToFile(BinaryData, *FilePath, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
}
}
