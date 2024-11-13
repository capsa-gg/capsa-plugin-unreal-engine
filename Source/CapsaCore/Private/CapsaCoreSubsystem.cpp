// Copyright Companion Group, Ltd. All Rights Reserved.

#include "CapsaCoreSubsystem.h"
#include "CapsaCore.h"
#include "Components/CapsaActorComponent.h"
#include "FunctionLibrary/CapsaCoreFunctionLibrary.h"
#include "Settings/CapsaSettings.h"

#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "Kismet/GameplayStatics.h"


/**
* Async task to create a FString that we can send over HTTP
* from a FBufferredLine TArray.
*/
class FMakeStringFromBufferTask : public FNonAbandonableTask
{
public:
    friend class FAutoDeleteAsyncTask<FMakeStringFromBufferTask>;

    FMakeStringFromBufferTask( TArray<FBufferedLine> InBuffer, FAsyncStringFromBufferCallback InCallbackFunction )
        : Buffer( MoveTemp(InBuffer) )
        , CallbackFunction( InCallbackFunction )
    {
    }

    void DoWork()
    {
        FString Log;
        for( const FBufferedLine& Line : Buffer )
        {
            // Construct the Time from the Seconds when the Line was added
            FTimespan TimeSpan = FTimespan::FromSeconds( Line.Time );
            FDateTime Time;
            Time += TimeSpan;
            FWideStringBuilderBase TimeStamp;
            // format: yyyy.mm.dd-hh.mm.ss:mil
            Time.ToString( TEXT( "%Y.%m.%d-%H.%M.%S:%s" ), TimeStamp );

            Log.Append( FString::Printf( TEXT("[%s]"), *TimeStamp ) );
            Log.Append( FString::Printf( TEXT( "[%d]" ), Line.Verbosity ) );
            Log.Append( FString::Printf( TEXT( "[%s]: " ), *Line.Category.Resolve().ToString() ) );
            Log.Append( Line.Data.Get() );
            Log.Append( LINE_TERMINATOR_ANSI );
        }

        CallbackFunction( Log );
    }

    FORCEINLINE TStatId GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT( FMakeStringFromBufferTask, STATGROUP_ThreadPoolAsyncTasks );
    }

private:
    TArray<FBufferedLine>           Buffer;
    FAsyncStringFromBufferCallback  CallbackFunction;
};



UCapsaCoreSubsystem::UCapsaCoreSubsystem()
    : Token( "" )
    , LogID( "" )
    , LinkWeb( "" )
    , Expiry( "" )
{
}

void UCapsaCoreSubsystem::Initialize( FSubsystemCollectionBase& Collection )
{
	Super::Initialize( Collection );

    UE_LOG( LogCapsaCore, Log, TEXT( "UCapsaCoreSubsystem::Initialize | Starting Up..." ) );

    RequestClientAuth();

    OnPostWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject( this, &UCapsaCoreSubsystem::OnPostWorldInit );
}

void UCapsaCoreSubsystem::Deinitialize()
{
    if( OnPostWorldInitializationHandle.IsValid() == true )
    {
        FWorldDelegates::OnPostWorldInitialization.Remove( OnPostWorldInitializationHandle );
    }

	Super::Deinitialize();
}

bool UCapsaCoreSubsystem::IsAuthenticated() const
{
    return ( Token.IsEmpty() == false ) && ( LogID.IsEmpty() == false );
}

void UCapsaCoreSubsystem::SendLog( TArray<FBufferedLine>& LogBuffer )
{
    FAsyncStringFromBufferCallback CallbackFunc = [this]( const FString& Log )
        {
            RequestSendLog( Log );
        };
    ( new FAutoDeleteAsyncTask<FMakeStringFromBufferTask>( MoveTemp( LogBuffer ), CallbackFunc ) )->StartBackgroundTask();
}

void UCapsaCoreSubsystem::RequestClientAuth()
{
    const UCapsaSettings* CapsaSettings = GetDefault<UCapsaSettings>();
    if( CapsaSettings == nullptr || CapsaSettings->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::RequestClientAuth | Failed to load CapsaSettings." ) );
        return;
    }

    FString AuthURL = CapsaSettings->GetCapsaBaseURL();
    if( AuthURL.IsEmpty() == true )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::RequestClientAuth | AuthURL is Empty!" ) );
        return;
    }
    AuthURL.Append( CapsaSettings->GetCapsaURLAuthSuffix() );

    FString AuthContent = TEXT( "{" );
    AuthContent.Append( TEXT( "\"key\":\"" ) );
    AuthContent.Append( CapsaSettings->GetCapsaAuthKey() );
    AuthContent.Append( TEXT( "\",\"platform\":\"" ) );
    AuthContent.Append( UCapsaCoreFunctionLibrary::GetPlatformString() );
    AuthContent.Append( TEXT( "\",\"type\":\"" ) );
    AuthContent.Append( UCapsaCoreFunctionLibrary::GetHostTypeString() );
    AuthContent.Append( TEXT( "\"}" ) );

    FHttpRequestRef ClientAuthRequest = FHttpModule::Get().CreateRequest();
    ClientAuthRequest->SetURL( AuthURL );
    ClientAuthRequest->SetVerb( "POST" );
    ClientAuthRequest->SetHeader( "Content-Type", "application/json" );
    ClientAuthRequest->SetContentAsString( AuthContent );
    ClientAuthRequest->OnProcessRequestComplete().BindUObject( this, &UCapsaCoreSubsystem::ClientAuthResponse );
    ClientAuthRequest->ProcessRequest();
}

void UCapsaCoreSubsystem::ClientAuthResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess )
{
    TSharedPtr<FJsonObject> JsonObject = ProcessResponse( TEXT( "UCapsaCoreSubsystem::ClientAuthResponse" ), Request, Response, bSuccess );

    if( JsonObject != nullptr && JsonObject.IsValid() == true )
    {
        Token = JsonObject->GetStringField( TEXT( "token" ) );
        LogID = JsonObject->GetStringField( TEXT( "log_id" ) );
        LinkWeb = JsonObject->GetStringField( TEXT( "link_web" ) );
        Expiry = JsonObject->GetStringField( TEXT( "expiry" ) );

        return;
    }

    UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::ClientAuthResponse | Failed to parse JSON." ) );
}

void UCapsaCoreSubsystem::RequestSendLog( const FString& Log )
{
    const UCapsaSettings* CapsaSettings = GetDefault<UCapsaSettings>();
    if( CapsaSettings == nullptr || CapsaSettings->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::RequestSendLog | Failed to load CapsaSettings." ) );
        return;
    }

    FString LogURL = CapsaSettings->GetCapsaBaseURL();
    LogURL.Append( CapsaSettings->GetCapsaURLLogChunkSuffix() );

    FString LogAuthHeader = TEXT( "Bearer " );
    LogAuthHeader.Append( Token );

    FHttpRequestRef LogRequest = FHttpModule::Get().CreateRequest();
    LogRequest->SetURL( LogURL );
    LogRequest->SetVerb( "POST" );
    LogRequest->SetHeader( "Authorization", LogAuthHeader );
    LogRequest->AppendToHeader( "Content-Type", "text/plain" );
    LogRequest->SetContentAsString( Log );
    LogRequest->OnProcessRequestComplete().BindUObject( this, &UCapsaCoreSubsystem::LogResponse );
    LogRequest->ProcessRequest();
}

void UCapsaCoreSubsystem::LogResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess )
{
    ProcessResponse( TEXT( "UCapsaCoreSubsystem::LogResponse" ), Request, Response, bSuccess );
}

TSharedPtr<FJsonObject> UCapsaCoreSubsystem::ProcessResponse( const FString& LogDetails, FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess )
{
    try
    {
        if( bSuccess == true )
        {
            // Deserialize the JSON
            TSharedPtr<FJsonObject> JsonObject = MakeShareable( new FJsonObject );
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create( Response->GetContentAsString() );

            try
            {
                if( FJsonSerializer::Deserialize( Reader, JsonObject ) == true )
                {
                    return JsonObject;
                }
            }
            catch( ... )
            {
                //UE_LOG( LogCapsaCore, Error, TEXT( "%s | Failed to Deserialize JSON" ), *LogDetails );
            }
        }
        else
        {
            UE_LOG( LogCapsaCore, Error, TEXT( "%s | HTTP Request Failed: %s." ), *LogDetails, ( Response == nullptr || Response.IsValid() == false ) ? TEXT( "Invalid Response Ptr" ) : *Response->GetContentAsString() );
        }
    }
    catch( ... )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "%s | HTTP Request Failed." ), *LogDetails );
    }

    return nullptr;
}

void UCapsaCoreSubsystem::OnPostWorldInit( UWorld* World, const UWorld::InitializationValues )
{
    UE_LOG( LogCapsaCore, Warning, TEXT( "UCapsaCoreSubsystem::OnPostLoadMapWithWorld | Post Map Load..." ) );

    if( World == nullptr || World->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::OnPostLoadMapWithWorld | World is Invalid!" ) );
        return;
    }

    if( World->GetNetMode() == NM_Client )
    {
        // Check for the CapsaActorComponent
        // If not found, Timer (or Event?)
        // Once we have a valid one, send OUR CapsaID via the RPC
        return;
    }

    // Fall through - some kind of Server (anything that's NOT an NM_Client is some kind of server)
    
    const UCapsaSettings* CapsaSettings = GetDefault<UCapsaSettings>();
    if( CapsaSettings == nullptr || CapsaSettings->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::OnPostLoadMapWithWorld | Failed to load CapsaSettings." ) );
        return;
    }

    if( CapsaSettings->GetShouldAutoAddCapsaComponent() == false )
    {
        return;
    }

    if( CapsaSettings->GetAutoAddClass() == nullptr )
    {
        return;
    }

    // TODO:
    // Add a Timer to check if the Class exists
    // Add a failure count (loop count)
    //

    TArray<AActor*> ActorsToAddComp;
    UGameplayStatics::GetAllActorsOfClass( World, CapsaSettings->GetAutoAddClass(), ActorsToAddComp );

    if( ActorsToAddComp.IsEmpty() == true )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::OnPostLoadMapWithWorld | Unable to add Capsa Component, no Actors of class %s found." ), *CapsaSettings->GetAutoAddClass()->GetName() );
        return;
    }

    for( AActor* Actor : ActorsToAddComp )
    {
        Actor->AddComponentByClass( UCapsaActorComponent::StaticClass(), false, FTransform(), false );
    }
}
