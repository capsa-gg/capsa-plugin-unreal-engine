// Copyright Companion Group, Ltd. All Rights Reserved.

#include "CapsaCore.h"
#include "FunctionLibrary/CapsaCoreFunctionLibrary.h"

#include "Serialization/ArchiveSaveCompressedProxy.h"
#include "Serialization/ArchiveLoadCompressedProxy.h"


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
        , LogExtension( TEXT( ".Log" ) )
        , CompressedExtension( TEXT( ".Capsa" ) )
    {
    }

    /**
    * Builds a Log string from the Buffer, with the format:
    * [Timestamp][LogVerbosity][LogCategory]: LogData\r\n
    * 
    * @return FString The generated Log from the Buffer.
    */
    FString                         MakeLogString()
    {
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
            Log.Append( LINE_TERMINATOR_ANSI );
        }

        return Log;
    }

    /**
    * Uses MakeLogString() to generate the Log.
    * Then compresses said log using GZip, ZLib or Oodle compression.
    * 
    * @param BinaryData The reference to the Binary Array to write to.
    */
    void                            MakeCompressedLogBinary( TArray<uint8>& BinaryData )
    {
        FString Log = MakeLogString();
        // Compression Options are: NAME_Zlib, NAME_Gzip or NAME_Oodle.
        FArchiveSaveCompressedProxy Compressor( BinaryData, NAME_Gzip );
        Compressor << Log;
        Compressor.Flush();
    }

    /**
    * Attempts to decompress the Binary Array into a FString.
    * Uses either GZip, ZLib or Oodle to decompress - ensure this matches what was used to compress!
    * 
    * @param Log The returned generated Log String, if successful.
    * @param BinaryData The source Binary Array to decompress and recover the Log string.
    * 
    * @return bool True if successfully decompressed and retrieved the log, otherwise false.
    */
    bool                            MakeLogStringFromCompressedBinary( FString& Log, const TArray<uint8>& BinaryData )
    {
        // Compression Options are: NAME_Zlib, NAME_Gzip or NAME_Oodle.
        FArchiveLoadCompressedProxy Decompressor( BinaryData, NAME_Gzip );

        if( Decompressor.GetError() == true )
        {
            return false;
        }

        Decompressor << Log;
        return true;
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
        FString FilePath = FPaths::ProjectLogDir() + FileName + LogExtension;
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
        FString FilePath = FPaths::ProjectLogDir() + FileName + CompressedExtension;
        return FFileHelper::SaveArrayToFile( BinaryData, *FilePath, &IFileManager::Get(), EFileWrite::FILEWRITE_Append );
    }

    /**
    * Attempts to Load a String from the provided FileName.
    * Uses the ProjectLogDir to read files from.
    * 
    * @param FileName The FileName to search for and read from.
    * @param LogLoaded The Log, if loaded from file.
    * 
    * @return bool True if successfully loaded, otherwise false.
    */
    bool                            LoadStringFromFile( const FString& FileName, FString& LogLoaded )
    {
        FString FilePath = FPaths::ProjectLogDir() + FileName + LogExtension;
        return FFileHelper::LoadFileToString( LogLoaded, *FilePath );
    }

    /**
    * Attempts to Load a Binary Array from the provided FileName.
    * Uses the ProjectLogDir to read files from.
    *
    * @param FileName The FileName to search for and read from.
    * @param BinaryDataLoaded The BinaryData, if loaded from file.
    *
    * @return bool True if successfully loaded, otherwise false.
    */
    bool                            LoadBinaryFromFile( const FString& FileName, TArray<uint8>& BinaryDataLoaded )
    {
        FString FilePath = FPaths::ProjectLogDir() + FileName + CompressedExtension;
        return FFileHelper::LoadFileToArray( BinaryDataLoaded, *FilePath );
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
*/
class FMakeStringFromBufferTask : public FCapsaAsyncTask<FAsyncStringFromBufferCallback>
{
public:
    friend class FAutoDeleteAsyncTask<FMakeStringFromBufferTask>;

    FMakeStringFromBufferTask( TArray<FBufferedLine> InBuffer, FAsyncStringFromBufferCallback InCallbackFunction )
        : FCapsaAsyncTask<FAsyncStringFromBufferCallback>( MoveTemp( InBuffer ), InCallbackFunction )
    {
    }

    void                            DoWork()
    {
        FString Log = MakeLogString();
        CallbackFunction( Log );
    }

    FORCEINLINE TStatId             GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT( FMakeStringFromBufferTask, STATGROUP_ThreadPoolAsyncTasks );
    }
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
        , bWriteToDisk( bInWriteToDisk )
    {
    }

    void                            DoWork()
    {
        FString Log = MakeLogString();
        if( bWriteToDisk == true )
        {
            SaveStringToFile( Log, LogID );
        }
        CallbackFunction( Log );
    }

    FORCEINLINE TStatId             GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT( FSaveStringFromBufferTask, STATGROUP_ThreadPoolAsyncTasks );
    }

protected:

    FString                         LogID;
    bool                            bWriteToDisk;
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

    FSaveCompressedStringFromBufferTask( FString InLogID, bool bInWriteToDisk, TArray<FBufferedLine> InBuffer, FAsyncBinaryFromBufferCallback InCallbackFunction )
        : FCapsaAsyncTask( MoveTemp( InBuffer ), InCallbackFunction )
        , LogID( InLogID )
        , bWriteToDisk( bInWriteToDisk )
    {
    }

    void                            DoWork()
    {
        TArray<uint8> CompressedLog;
        MakeCompressedLogBinary( CompressedLog );
        if( bWriteToDisk == true )
        {
            SaveBinaryToFile( CompressedLog, LogID );
        }
        
        CallbackFunction( CompressedLog );
    }

    FORCEINLINE TStatId             GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT( FSaveCompressedStringFromBufferTask, STATGROUP_ThreadPoolAsyncTasks );
    }

protected:

    FString                         LogID;
    bool                            bWriteToDisk;
};

/**
* Async task to create a FString that we can send over HTTP
* load from a File.
* If bInCompress is true, will use GZip, ZLib or Oodle decompression 
* and read the archive from file.
*/
class FLoadStringFromFileTask : public FCapsaAsyncTask<FAsyncStringFromBufferCallback>
{
public:
    friend class FAutoDeleteAsyncTask<FLoadStringFromFileTask>;

    FLoadStringFromFileTask( FString InLogID, bool bInDecompress, TArray<FBufferedLine> InBuffer, FAsyncStringFromBufferCallback InCallbackFunction )
        : FCapsaAsyncTask( MoveTemp( InBuffer ), InCallbackFunction )
        , LogID( InLogID )
        , bDecompress( bInDecompress )
    {
    }

    void                            DoWork()
    {
        FString Log;
        if( bDecompress == true )
        {
            TArray<uint8> DecompressedLog;
            LoadBinaryFromFile( LogID, DecompressedLog );
            MakeLogStringFromCompressedBinary( Log, DecompressedLog );
        }
        else
        {
            LoadStringFromFile( LogID, Log );
        }
        CallbackFunction( Log );
    }

    FORCEINLINE TStatId             GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT( FLoadStringFromFileTask, STATGROUP_ThreadPoolAsyncTasks );
    }

protected:

    FString                         LogID;
    bool                            bDecompress;
};
