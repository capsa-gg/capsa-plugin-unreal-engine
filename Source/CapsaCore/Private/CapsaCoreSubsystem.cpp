// Copyright Companion Group, Ltd. All Rights Reserved.

#include "CapsaCoreSubsystem.h"
#include "CapsaCore.h"
#include "Components/CapsaActorComponent.h"
#include "FunctionLibrary/CapsaCoreFunctionLibrary.h"
#include "Settings/CapsaSettings.h"

#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"


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
            FDateTime Time = FDateTime::FromUnixTimestampDecimal( Line.Time );
            FWideStringBuilderBase TimeStamp;
            // format: yyyy.mm.dd-hh.mm.ss:mil
            Log.Append( FString::Printf( TEXT( "[%s]" ), *Time.ToString( TEXT( "%Y.%m.%d-%H.%M.%S.%s" ) ) ) );
            Log.Append( FString::Printf( TEXT( "[%s]" ), *UCapsaCoreFunctionLibrary::GetLogVerbosityString( Line.Verbosity ) ) );
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
    , CapsaActorComponent( nullptr )
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

    FGameModeEvents::GameModePostLoginEvent.RemoveAll( this );
    FGameModeEvents::GameModeLogoutEvent.RemoveAll( this );

	Super::Deinitialize();
}

bool UCapsaCoreSubsystem::IsAuthenticated() const
{
    return ( Token.IsEmpty() == false ) && ( LogID.IsEmpty() == false );
}

FString UCapsaCoreSubsystem::GetLogID() const
{
    return LogID;
}

bool UCapsaCoreSubsystem::RegisterLinkedLogID( const FString& LinkedLogID )
{
    // Don't link with self.
    if( LogID.Equals( LinkedLogID ) == true )
    {
        return false;
    }

    if( LinkedLogIDs.Contains( LinkedLogID ) == true )
    {
        return false;
    }

    LinkedLogIDs.Add( LinkedLogID );
    return true;
}

bool UCapsaCoreSubsystem::UnregisterLinkedLogID( const FString& LinkedLogID )
{
    // Don't link with self.
    if( LogID.Equals( LinkedLogID ) == true )
    {
        return false;
    }

    if( LinkedLogIDs.Contains( LinkedLogID ) == false )
    {
        return false;
    }

    LinkedLogIDs.Remove( LinkedLogID );
    return true;
}

void UCapsaCoreSubsystem::SendLog( TArray<FBufferedLine>& LogBuffer )
{
    FAsyncStringFromBufferCallback CallbackFunc = [this]( const FString& Log )
        {
            RequestSendLog( Log );
        };
    ( new FAutoDeleteAsyncTask<FMakeStringFromBufferTask>( MoveTemp( LogBuffer ), CallbackFunc ) )->StartBackgroundTask();
}

FString UCapsaCoreSubsystem::GetCapsaLogURL( const FString& InLogID )
{
    const UCapsaSettings* CapsaSettings = GetDefault<UCapsaSettings>();
    if( CapsaSettings == nullptr || CapsaSettings->IsValidLowLevelFast() == false )
    {
        return "";
    }

    FString LogURL = CapsaSettings->GetProtocol();
    LogURL.Append( CapsaSettings->GetWebPrefix() );
    LogURL.Append( CapsaSettings->GetCapsaBaseURL() );
    //LogURL.Append( CapsaSettings->GetCapsaURLLogSuffix() );   // api needs log/ and web needs logs/
    LogURL.Append( TEXT( "logs/" ) );
    LogURL.Append( InLogID );

    return LogURL;
}

void UCapsaCoreSubsystem::RequestClientAuth()
{
    const UCapsaSettings* CapsaSettings = GetDefault<UCapsaSettings>();
    if( CapsaSettings == nullptr || CapsaSettings->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::RequestClientAuth | Failed to load CapsaSettings." ) );
        return;
    }

    FString AuthURL = CapsaSettings->GetProtocol();
    AuthURL.Append( CapsaSettings->GetAPIPrefix() );
    AuthURL.Append( CapsaSettings->GetCapsaBaseURL() );
    if( CapsaSettings->GetCapsaBaseURL().IsEmpty() == true )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::RequestClientAuth | Base URL is Empty!" ) );
        return;
    }

    AuthURL.Append( CapsaSettings->GetCapsaURLAPIPath() );
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

        // TODO: Change this URL Append to use "link_web" when it is implemented.
        FString LogURL = GetCapsaLogURL( LogID );
        UE_LOG( LogCapsaCore, Log, TEXT( "Capsa ID: %s | CapsaLogURL: %s" ), *LogID, *LogURL );
        // TODO_END

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

    FString LogURL = CapsaSettings->GetProtocol();
    LogURL.Append( CapsaSettings->GetAPIPrefix() );
    LogURL.Append( CapsaSettings->GetCapsaBaseURL() );
    LogURL.Append( CapsaSettings->GetCapsaURLAPIPath() );
    LogURL.Append( CapsaSettings->GetCapsaURLLogSuffix() );
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

void UCapsaCoreSubsystem::RequestSendMetadata()
{
    // TODO: Generate Metadata and send HTTP.
}

void UCapsaCoreSubsystem::MetadataResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess )
{
    ProcessResponse( TEXT( "UCapsaCoreSubsystem::MetadataResponse" ), Request, Response, bSuccess );
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
    if( World == nullptr || World->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::OnPostLoadMapWithWorld | World is Invalid!" ) );
        return;
    }

    if( World->GetNetMode() == NM_Client )
    {
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
    
    FGameModeEvents::GameModePostLoginEvent.AddUObject( this, &UCapsaCoreSubsystem::OnPlayerLoggedIn );
    FGameModeEvents::GameModeLogoutEvent.AddUObject( this, &UCapsaCoreSubsystem::OnPlayerLoggedOut );
}

void UCapsaCoreSubsystem::OnPlayerLoggedIn( AGameModeBase* GameMode, APlayerController* Player )
{
    const UCapsaSettings* CapsaSettings = GetDefault<UCapsaSettings>();
    if( CapsaSettings == nullptr || CapsaSettings->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::OnPlayerLoggedIn | Failed to load CapsaSettings." ) );
        return;
    }

    TArray<AActor*> ActorsToAddComp;
    UGameplayStatics::GetAllActorsOfClass( GameMode, CapsaSettings->GetAutoAddClass(), ActorsToAddComp );

    if( ActorsToAddComp.IsEmpty() == true )
    {
        UE_LOG( LogCapsaCore, Log, TEXT( "UCapsaCoreSubsystem::OnPlayerLoggedIn | Unable to add Capsa Component, no Actors of class %s found." ), *CapsaSettings->GetAutoAddClass()->GetName() );
        return;
    }

    // Once we have a valid one, send OUR CapsaID via the RPC
    for( AActor* Actor : ActorsToAddComp )
    {
        if( Actor->FindComponentByClass<UCapsaActorComponent>() != nullptr )
        {
            // Don't add again, if we already have a UCapsaActorComponent on this class.
            continue;
        }

        UActorComponent* ActorComponent = Actor->AddComponentByClass( UCapsaActorComponent::StaticClass(), false, FTransform(), false );
        if( CapsaActorComponent.IsValid() == false )
        {
            // Store a Weak Reference to the first made Actor Component.
            CapsaActorComponent = Cast<UCapsaActorComponent>( ActorComponent );
        }
    }
}

void UCapsaCoreSubsystem::OnPlayerLoggedOut( AGameModeBase* GameMode, AController* Controller )
{
}

void UCapsaCoreSubsystem::OpenClientLogInBrowser()
{
    UCapsaCoreSubsystem* CapsaCore = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();

    if( CapsaCore == nullptr || CapsaCore->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "Unable to open Client Log in Browser: CapsaCore Subsystem is invalid." ) );
        return;
    }

    FString LogURL = CapsaCore->GetCapsaLogURL( CapsaCore->GetLogID() );
    UCapsaCoreSubsystem::OpenBrowser( LogURL );
}

void UCapsaCoreSubsystem::OpenServerLogInBrowser()
{
    UCapsaCoreSubsystem* CapsaCore = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();

    if( CapsaCore == nullptr || CapsaCore->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "Unable to open Server Log in Browser: CapsaCore Subsystem is invalid." ) );
        return;
    }

    if( CapsaCore->CapsaActorComponent.IsValid() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "Unable to open Server Log in Browser: Unable to GET Server LogID." ) );
        return;
    }

    FString LogURL = CapsaCore->GetCapsaLogURL( CapsaCore->CapsaActorComponent->CapsaServerId );
    UCapsaCoreSubsystem::OpenBrowser( LogURL );
}

void UCapsaCoreSubsystem::OpenBrowser( FString URL )
{
    FPlatformProcess::LaunchURL( *URL, NULL, NULL );
}

static FAutoConsoleCommand CVarCapsaViewClientLog(
    TEXT( "Capsa.ViewClientLog" ),
    TEXT( "Requests the Operating System to open the default browser " )
    TEXT( "and open the Capsa Log URL for the current session." ),
    FConsoleCommandDelegate::CreateStatic( UCapsaCoreSubsystem::OpenClientLogInBrowser ),
    ECVF_Cheat );

static FAutoConsoleCommand CVarCapsaViewServerLog(
    TEXT( "Capsa.ViewServerLog" ),
    TEXT( "Requests the Operating System to open the default browser " )
    TEXT( "and open the Capsa Log URL for the connected server in the current session." ),
    FConsoleCommandDelegate::CreateStatic( UCapsaCoreSubsystem::OpenServerLogInBrowser ),
    ECVF_Cheat );
