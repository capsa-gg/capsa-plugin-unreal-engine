// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "CoreMinimal.h"

namespace CapsaLogOperations
{
inline FString DefaultUncompressedLogExtension = TEXT(".capsa.log"); ///< Default log extension for uncompressed logs
inline FString DefaultCompressedLogExtension = TEXT(".capsa.log.zlib"); ///< Default log extension for compressed logs

/// Builds a Log string from the Buffer, with the format:
/// [Timestamp][LogVerbosity][LogCategory]: LogData\n
/// @return FString The generated Log from the Buffer.
FString MakeLogString (const TArray<FBufferedLine>& Buffer);

/// Uses MakeLogString() to generate the Log. Then compresses said log using GZip, ZLib or Oodle compression.
/// @param UncompressedLog The reference to the Uncompressed Log FString to write to.
/// @param BinaryData The reference to the Binary Array to write to.
/// @return bool True if compression was successful.
bool MakeCompressedLogBinary(const FString& UncompressedLog, TArray<uint8>& BinaryData);

/// Attempts to save the provided Log String to a file with the provided FileName.
/// Uses the ProjectLogDir folder to output the file to.
/// @param LogToSave The Source FString Log to save to file.
/// @param FileName The name of the file to save.
/// @param FileExtension The file extension to use for the file including leading comma.
/// @return bool True if successfully written to file, otherwise false.
bool SaveStringToFile(const FString& LogToSave, const FString& FileName, const FString& FileExtension);

/// Attempts to save the provided BinaryData to a file with the provided FileName. Uses the ProjectLogDir folder to output the file to.
/// @param BinaryData The Source Binary Array to save to file.
/// @param FileName The name of the file to save.
/// @param FileExtension The file extension to use for the file including leading comma.
/// @return bool True if successfully written to file, otherwise false.
bool SaveBinaryToFile(TArray<uint8> BinaryData, const FString& FileName, const FString& FileExtension);
}
