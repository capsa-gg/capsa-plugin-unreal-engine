// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "CapsaCore.h"
#include "FunctionLibrary/CapsaCoreFunctionLibrary.h"


typedef TFunction<void( const FString& )> FAsyncStringFromBufferCallback;
typedef TFunction<void( const TArray<uint8>& )> FAsyncBinaryFromBufferCallback;


/**
* Base Capsa Async Task.
* Stores the Buffer and Callback function. Also contains base helper methods like
* those to construct a single Log String from the Buffer.
*/
template <typename CallbackType>
class FCapsaAsyncTask : public FNonAbandonableTask
{
public:
    friend class FAutoDeleteAsyncTask<FCapsaAsyncTask>;

    FCapsaAsyncTask( TArray<FBufferedLine> InBuffer, CallbackType InCallbackFunction )
        : Buffer( MoveTemp( InBuffer ) )
        , CallbackFunction( InCallbackFunction )
        , LogExtension( TEXT( ".capsa.log" ) )
        , CompressedExtension( TEXT( ".capsa.log.zlib" ) )
    {
    }

    /**
    * Builds a Log string from the Buffer, with the format:
    * [Timestamp][LogVerbosity][LogCategory]: LogData\n
    * 
    * @return FString The generated Log from the Buffer.
    */
    FString                         MakeLogString()
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(MakeLogString);
        
        FString Log;
        for( const FBufferedLine& Line : Buffer )
        {
            // Construct the Time from the Seconds when the Line was added
            FDateTime Time = FDateTime::FromUnixTimestampDecimal( Line.Time );
            FWideStringBuilderBase TimeStamp;
            // format: yyyy.mm.dd-hh.mm.ss:mil
            Log.Append( FString::Printf( TEXT( "[%s]" ), *Time.ToString( TEXT( "%Y.%m.%d-%H.%M.%S.%s" ) ) ) );
            Log.Append( FString::Printf( TEXT( "[%s]" ), *UCapsaCoreFunctionLibrary::GetLogVerbosityString( Line.Verbosity ) ) );
            Log.Append( FString::Printf( TEXT( "[%s]: " ), *Line.Category.Resolve().ToString() ) );
            Log.Append( Line.Data.Get() );
            Log.Append( LINE_TERMINATOR_ANSI ); // Use lf ending on all platforms
        }

        return Log;
    }

    /**
    * Uses MakeLogString() to generate the Log.
    * Then compresses said log using GZip, ZLib or Oodle compression.
    *
    * @param UncompressedLog The reference to the Uncompressed Log FString to write to.
    * @param BinaryData The reference to the Binary Array to write to.
    * @return bool True if compression was successful.
    */
    bool                            MakeCompressedLogBinary( FString& UncompressedLog, TArray<uint8>& BinaryData )
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(MakeCompressedLogBinary);

        UE_LOG( LogCapsaCore, VeryVerbose, TEXT("FCapsaAsyncTask::MakeCompressedLogBinary | Start compression") )
        
        // Get log string, uncompressed
        UncompressedLog = MakeLogString();
        UE_LOG( LogCapsaCore, VeryVerbose, TEXT("FCapsaAsyncTask::MakeCompressedLogBinary | Uncompressed log length: %d"), UncompressedLog.Len() );
        
        // Convert log string to uint8*
        TArray<uint8> UncompressedLogBytes;
        uint64 Utf8Length = FPlatformString::ConvertedLength<UTF8CHAR>(*UncompressedLog, UncompressedLog.Len());
        UncompressedLogBytes.SetNumUninitialized(Utf8Length);
        FPlatformString::Convert((UTF8CHAR*)UncompressedLogBytes.GetData(), UncompressedLogBytes.Num(), *UncompressedLog, UncompressedLog.Len());
        
        
        // Reserve memory for compressed data, setting 4/3 in case compressed turns out longer
        BinaryData.SetNumUninitialized( UncompressedLog.Len() * 4/3 );
        int32 CompressedSize = BinaryData.Num();
        UE_LOG( LogCapsaCore, VeryVerbose, TEXT("FCapsaAsyncTask::FCapsaAsyncTask | Utf8Length: %llu"), Utf8Length )
        
        // Compress data 
        const bool bSuccess = FCompression::CompressMemory(
            NAME_Zlib,
            BinaryData.GetData(),
            CompressedSize,
            UncompressedLogBytes.GetData(),
            UncompressedLogBytes.Num()
        );
        
        UE_LOG( LogCapsaCore, Verbose, TEXT( "FCapsaAsyncTask::MakeCompressedLogBinary | Success: %d, compressed size: %d" ), bSuccess, CompressedSize );

        return bSuccess;
    }

    /**
    * Attempts to save the provided Log String to a file with the provided FileName.
    * Uses the ProjectLogDir folder to output the file to.
    * 
    * @param LogToSave The Source FString Log to save to file.
    * @param FileName The name of the file to save.
    * 
    * @return bool True if successfully written to file, otherwise false.
    */
    bool                            SaveStringToFile( const FString& LogToSave, const FString& FileName )
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(SaveStringToFile);
        
        FString FilePath = FPaths::ProjectLogDir() + FileName + LogExtension;
        
        UE_LOG( LogCapsaCore, Verbose, TEXT( "FCapsaAsyncTask::SaveStringToFile | Attempting to write/append to: %s" ), *FilePath );
        
        return FFileHelper::SaveStringToFile( LogToSave, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append );
    }

    /**
    * Attempts to save the provided BinaryData to a file with the provided FileName.
    * Uses the ProjectLogDir folder to output the file to.
    *
    * @param BinaryData The Source Binary Array to save to file.
    * @param FileName The name of the file to save.
    *
    * @return bool True if successfully written to file, otherwise false.
    */
    bool                            SaveBinaryToFile( TArray<uint8> BinaryData, const FString& FileName )
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(SaveBinaryToFile);
        
        FString CapsaCompressedDirectory = TEXT( "CapsaCompressedChunks/" ) + FileName + TEXT( "/" );
        FString FilePath = FPaths::ProjectLogDir() + CapsaCompressedDirectory + FDateTime::Now().ToString(TEXT( "%Y-%m-%dT%H.%M.%S.%s" )) + CompressedExtension;

        UE_LOG( LogCapsaCore, Verbose, TEXT( "FCapsaAsyncTask::SaveBinaryToFile | Attempting to write to: %s" ), *FilePath );
        
        return FFileHelper::SaveArrayToFile( BinaryData, *FilePath, &IFileManager::Get(), EFileWrite::FILEWRITE_Append );
    }
    
    void                            DoWork()
    {
        // Default does nothing.
    }

    FORCEINLINE TStatId             GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT( FCapsaAsyncTask, STATGROUP_ThreadPoolAsyncTasks );
    }

protected:

    TArray<FBufferedLine>           Buffer;
    CallbackType                    CallbackFunction;
    const FString                   LogExtension;
    const FString                   CompressedExtension;
};

/**
* Async task to create a FString that we can send over HTTP
* from a FBufferredLine TArray.
* And then save this Raw String to File.
*/
class FSaveStringFromBufferTask : public FCapsaAsyncTask<FAsyncStringFromBufferCallback>
{
public:
    friend class FAutoDeleteAsyncTask<FSaveStringFromBufferTask>;

    FSaveStringFromBufferTask( FString InLogID, bool bInWriteToDisk, TArray<FBufferedLine> InBuffer, FAsyncStringFromBufferCallback InCallbackFunction )
        : FCapsaAsyncTask<FAsyncStringFromBufferCallback>( MoveTemp( InBuffer ), InCallbackFunction )
        , LogID( InLogID )
        , bWriteToDiskPlain( bInWriteToDisk )
    {
    }

    void                            DoWork()
    {
        FString Log = MakeLogString();
        if( bWriteToDiskPlain == true )
        {
            if( SaveStringToFile( Log, LogID ) == false )
            {
                UE_LOG( LogCapsaCore, Warning, TEXT( "Failed to write plain text file to disk" ) )
            }
        }
        CallbackFunction( Log );
    }

    FORCEINLINE TStatId             GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT( FSaveStringFromBufferTask, STATGROUP_ThreadPoolAsyncTasks );
    }

protected:

    FString                         LogID;
    bool                            bWriteToDiskPlain;
};

/**
* Async task to create a Binary Array that we can send over HTTP
* from a FBufferredLine TArray.
* And then save this compressed Binary Array to File.
*/
class FSaveCompressedStringFromBufferTask : public FCapsaAsyncTask<FAsyncBinaryFromBufferCallback>
{
public:
    friend class FAutoDeleteAsyncTask<FSaveCompressedStringFromBufferTask>;

    FSaveCompressedStringFromBufferTask( FString InLogID, bool bInWriteToDiskPlain, bool bInWriteToDiskCompressed, TArray<FBufferedLine> InBuffer, FAsyncBinaryFromBufferCallback InCallbackFunction )
        : FCapsaAsyncTask( MoveTemp( InBuffer ), InCallbackFunction )
        , LogID( InLogID )
        , bWriteToDiskPlain( bInWriteToDiskPlain )
        , bWriteToDiskCompressed( bInWriteToDiskCompressed )
    {
    }

    void                            DoWork()
    {
        FString Log;
        TArray<uint8> CompressedLog;

        // Compress data
        if( MakeCompressedLogBinary( Log, CompressedLog ) == false)
        {
            UE_LOG( LogCapsaCore, Warning, TEXT( "FSaveCompressedStringFromBufferTask::DoWork | Failed to compress log binary" ) )
        } else
        {
            UE_LOG( LogCapsaCore, VeryVerbose, TEXT( "FSaveCompressedStringFromBufferTask::DoWork | Compressed log binary, length: %hhu" ), *CompressedLog.GetData() )
            
            // Save compressed file to disk
            if( bWriteToDiskCompressed == true )
            {
                if( SaveBinaryToFile( CompressedLog, LogID ) == false )
                {
                    UE_LOG( LogCapsaCore, Warning, TEXT( "FSaveCompressedStringFromBufferTask::DoWork | Failed to write compressed file to disk" ) )
                }
            }
        }

        // Save plain text to disk
        if( bWriteToDiskPlain == true )
        {
            if( SaveStringToFile( Log, LogID ) == false )
            {
                UE_LOG( LogCapsaCore, Warning, TEXT( "FSaveCompressedStringFromBufferTask::DoWork | Failed to write plain text file to disk" ) )
            } 
        }
        
        CallbackFunction( CompressedLog );
    }

    FORCEINLINE TStatId             GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT( FSaveCompressedStringFromBufferTask, STATGROUP_ThreadPoolAsyncTasks );
    }

protected:

    FString                         LogID;
    bool                            bWriteToDiskPlain;
    bool                            bWriteToDiskCompressed;
};

