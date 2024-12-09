// Copyright Companion Group, Ltd. Made available under the MIT license

#include "CapsaCoreSubsystem.h"
#include "CapsaCore.h"
#include "CapsaCoreAsync.h"
#include "Components/CapsaActorComponent.h"
#include "FunctionLibrary/CapsaCoreFunctionLibrary.h"
#include "Settings/CapsaSettings.h"

#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"



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
    RequestSendMetadata();
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
    
    if( LinkedLogIDs.IsEmpty() == false )
    {
        RequestSendMetadata();
    }

    return true;
}

void UCapsaCoreSubsystem::SendLog( TArray<FBufferedLine>& LogBuffer )
{
    const UCapsaSettings* CapsaSettings = GetDefault<UCapsaSettings>();
    if( CapsaSettings == nullptr || CapsaSettings->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::SendLog | Failed to load CapsaSettings." ) );
        return;
    }

    if( CapsaSettings->GetUseCompression() == true )
    {
        FAsyncBinaryFromBufferCallback CallbackFunc = [this]( const TArray<uint8>& CompressedLog )
            {
                RequestSendCompressedLog( CompressedLog );
            };
        // Example AsyncTask to attempt to SAVE the file using the LogID (as filename), whether compressed or not, then fire the Callback.
        // This requires a Binary Callback, not an FString
        ( new FAutoDeleteAsyncTask<FSaveCompressedStringFromBufferTask>( LogID, CapsaSettings->GetWriteToDiskPlain(), CapsaSettings->GetWriteToDiskCompressed(), MoveTemp(LogBuffer), CallbackFunc) )->StartBackgroundTask();
    }
    else // bUseCompression == false
    {
        FAsyncStringFromBufferCallback CallbackFunc = [this]( const FString& Log )
            {
                RequestSendLog( Log );
            };
        // These all require an FString Callback.
        // Example AsyncTask to generate a Log and Optionally write it to Disk, then fire the Callback.
        ( new FAutoDeleteAsyncTask<FSaveStringFromBufferTask>( LogID, CapsaSettings->GetWriteToDiskPlain(), MoveTemp( LogBuffer ), CallbackFunc ) )->StartBackgroundTask();
    }
}

void UCapsaCoreSubsystem::RequestClientAuth()
{
    const UCapsaSettings* CapsaSettings = GetDefault<UCapsaSettings>();
    if( CapsaSettings == nullptr || CapsaSettings->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::RequestClientAuth | Failed to load CapsaSettings." ) );
        return;
    }
    if (CapsaSettings->GetCapsaServerURL().IsEmpty() == true)
    {
        UE_LOG( LogCapsaCore, Error, TEXT("UCapsaCoreSubsystem::RequestClientAuth | Base URL is Empty!") );
        return;
    }

    FString AuthURL = CapsaSettings->GetServerEndpointClientAuth();

    TSharedPtr<FJsonObject> JsonObject = MakeShareable( new FJsonObject );
    JsonObject->SetStringField( TEXT( "key" ), CapsaSettings->GetCapsaEnvironmentKey() );
    JsonObject->SetStringField( TEXT( "platform" ), UCapsaCoreFunctionLibrary::GetPlatformString() );
    JsonObject->SetStringField( TEXT( "type" ), UCapsaCoreFunctionLibrary::GetHostTypeString() );
    FString AuthContent;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create( &AuthContent, 0 );
    FJsonSerializer::Serialize( JsonObject.ToSharedRef(), Writer );

    FHttpRequestRef ClientAuthRequest = FHttpModule::Get().CreateRequest();
    ClientAuthRequest->SetURL( AuthURL );
    ClientAuthRequest->SetVerb( "POST" );
    ClientAuthRequest->SetHeader( "Content-Type", "application/json" );
    ClientAuthRequest->SetContentAsString( AuthContent );
    ClientAuthRequest->OnProcessRequestComplete().BindUObject( this, &UCapsaCoreSubsystem::ClientAuthResponse );
    ClientAuthRequest->ProcessRequest();

    UE_LOG( LogCapsaCore, Log, TEXT("UCapsaCoreSubsystem::RequestClientAuth | Authentication request sent") );
}

void UCapsaCoreSubsystem::ClientAuthResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess )
{
    UE_LOG( LogCapsaCore, Log, TEXT("UCapsaCoreSubsystem::ClientAuthResponse | Authentication resonse received sent") );

    TSharedPtr<FJsonObject> JsonObject = ProcessResponse( TEXT( "UCapsaCoreSubsystem::ClientAuthResponse" ), Request, Response, bSuccess );

    if( JsonObject != nullptr && JsonObject.IsValid() == true )
    {
        Token = JsonObject->GetStringField( TEXT( "token" ) );
        LogID = JsonObject->GetStringField( TEXT( "log_id" ) );
        LinkWeb = JsonObject->GetStringField( TEXT( "link_web" ) );
        Expiry = JsonObject->GetStringField( TEXT( "expiry" ) );

        UE_LOG( LogCapsaCore, Log, TEXT( "Capsa ID: %s | CapsaLogURL: %s" ), *LogID, *LinkWeb);

        return;
    }

    UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::ClientAuthResponse | Failed to parse JSON." ) );
}

void UCapsaCoreSubsystem::RequestSendLog( const FString& Log )
{
    UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaCoreSubsystem::RequestSendLog | Sending log chunk without compression") );

    const UCapsaSettings* CapsaSettings = GetDefault<UCapsaSettings>();
    if( CapsaSettings == nullptr || CapsaSettings->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::RequestSendLog | Failed to load CapsaSettings." ) );
        return;
    }

    FString LogURL = CapsaSettings->GetServerEndpointClientLogChunk();

    FString LogAuthHeader = TEXT( "Bearer " );
    LogAuthHeader.Append( Token );

    FHttpRequestRef LogRequest = FHttpModule::Get().CreateRequest();
    LogRequest->SetURL( LogURL );
    LogRequest->SetVerb( "POST" );
    LogRequest->SetHeader( "Authorization", LogAuthHeader );
    LogRequest->SetHeader( "Content-Type", "text/plain" );
    LogRequest->SetContentAsString( Log );
    LogRequest->OnProcessRequestComplete().BindUObject( this, &UCapsaCoreSubsystem::LogResponse );
    LogRequest->ProcessRequest();
}

void UCapsaCoreSubsystem::RequestSendCompressedLog( const TArray<uint8>& CompressedLog )
{
    UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaCoreSubsystem::RequestSendCompressedLog | Sending log chunk with compression") );

    const UCapsaSettings* CapsaSettings = GetDefault<UCapsaSettings>();
    if( CapsaSettings == nullptr || CapsaSettings->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::RequestSendCompressedLog | Failed to load CapsaSettings." ) );
        return;
    }

    FString LogURL = *CapsaSettings->GetServerEndpointClientLogChunk();

    FString LogAuthHeader = TEXT( "Bearer " );
    LogAuthHeader.Append( Token );

    FHttpRequestRef LogRequest = FHttpModule::Get().CreateRequest();
    LogRequest->SetURL( LogURL );
    LogRequest->SetVerb( "POST" );
    LogRequest->SetHeader( "Authorization", LogAuthHeader );
    LogRequest->SetHeader( "Content-Type", "application/zlib" );
    LogRequest->SetContent( CompressedLog );
    LogRequest->OnProcessRequestComplete().BindUObject( this, &UCapsaCoreSubsystem::LogResponse );
    LogRequest->ProcessRequest();
}

void UCapsaCoreSubsystem::LogResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess )
{
    UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaCoreSubsystem::LogResponse | Log chunk stored") );

    ProcessResponse( TEXT( "UCapsaCoreSubsystem::LogResponse" ), Request, Response, bSuccess );
}

void UCapsaCoreSubsystem::RequestSendMetadata()
{
    UE_LOG (LogCapsaCore, Verbose, TEXT("UCapsaCoreSubsystem::RequestSendMetadata | Storing metadata") );

    const UCapsaSettings* CapsaSettings = GetDefault<UCapsaSettings>();
    if( CapsaSettings == nullptr || CapsaSettings->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::RequestSendMetadata | Failed to load CapsaSettings." ) );
        return;
    }

    FString MetadataURL = CapsaSettings->GetServerEndpointClientLogMetadata();

    FString LogAuthHeader = TEXT( "Bearer " );
    LogAuthHeader.Append( Token );

    FHttpRequestRef LogRequest = FHttpModule::Get().CreateRequest();
    LogRequest->SetURL( MetadataURL );
    LogRequest->SetVerb( "POST" );
    LogRequest->SetHeader( "Authorization", LogAuthHeader );
    LogRequest->AppendToHeader( "Content-Type", "application/json" );
    
    TSharedPtr<FJsonObject> JsonObject = MakeShareable( new FJsonObject );
    TArray<TSharedPtr<FJsonValue>> LinkedLogs;
    for( FString LinkedLogID : LinkedLogIDs )
    {
        TSharedPtr<FJsonValueString> JsonLinkedLogID = MakeShareable( new FJsonValueString( LinkedLogID ) );
        LinkedLogs.Add( JsonLinkedLogID );
    }
    JsonObject->SetArrayField( TEXT( "log_links" ), LinkedLogs );
    
    // FIXME: Add support
    TArray<TSharedPtr<FJsonValue>> AdditionalMetadata;
    // Loop/Add Metadata
    JsonObject->SetArrayField( TEXT( "additional_metadata" ), AdditionalMetadata );
    
    FString MetadataContent;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create( &MetadataContent, 0 );
    FJsonSerializer::Serialize( JsonObject.ToSharedRef(), Writer );
    
    LogRequest->SetContentAsString( MetadataContent );
    LogRequest->OnProcessRequestComplete().BindUObject( this, &UCapsaCoreSubsystem::LogResponse );
    LogRequest->ProcessRequest();
}

void UCapsaCoreSubsystem::MetadataResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess )
{
    UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaCoreSubsystem::MetadataResponse | Metadata stored") );

    ProcessResponse( TEXT( "UCapsaCoreSubsystem::MetadataResponse" ), Request, Response, bSuccess );
}

TSharedPtr<FJsonObject> UCapsaCoreSubsystem::ProcessResponse( const FString& RequestName, FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess )
{
    // Exit early if request failed
    if( !bSuccess == true )
    {
        UE_LOG( LogCapsaCore, Error, TEXT("%s | HTTP Request Failed: %s."), *RequestName, (Response == nullptr || Response.IsValid() == false) ? TEXT("Invalid Response Ptr") : *Response->GetContentAsString() );
        return nullptr;
    }

    if( Response->GetResponseCode() > 299 )
    {
        UE_LOG( LogCapsaCore, Warning, TEXT("%s | Received non-2xx response code %d: %s."), *RequestName, Response->GetResponseCode(), (Response == nullptr || Response.IsValid() == false) ? TEXT("Invalid Response Ptr") : *Response->GetContentAsString() );
    }

    if( Response->GetContentAsString().IsEmpty() ) 
    {
        UE_LOG( LogCapsaCore, VeryVerbose, TEXT("%s | Not deserializing JSON due to empty response body"), *RequestName );
        return nullptr;
    }

    // Deserialize the JSON
    TSharedPtr<FJsonObject> JsonObject = MakeShareable( new FJsonObject );
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create( Response->GetContentAsString() );

    if( FJsonSerializer::Deserialize( Reader, JsonObject ) == true )
    {
        return JsonObject;
    }

    UE_LOG( LogCapsaCore, Error, TEXT( "%s | Failed to deserialize JSON" ), *RequestName);

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

    UE_LOG( LogCapsaCore, Log, TEXT("UCapsaCoreSubsystem::OnPostWorldInit | GameModePostLoginEvent and GameModeLogoutEvent delegates added") );
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

            UE_LOG(LogCapsaCore, VeryVerbose, TEXT("UCapsaCoreSubsystem::OnPlayerLoggedIn | UCapsaActorComponent reference stored"));
        }
    }

    UE_LOG(LogCapsaCore, VeryVerbose, TEXT("UCapsaCoreSubsystem::OnPlayerLoggedIn | Finished"));
}

void UCapsaCoreSubsystem::OnPlayerLoggedOut( AGameModeBase* GameMode, AController* Controller )
{
}

void UCapsaCoreSubsystem::OpenClientLogInBrowser()
{
    UCapsaCoreSubsystem* CapsaCore = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();

    UE_LOG(LogCapsaCore, VeryVerbose, TEXT("UCapsaCoreSubsystem::OpenClientLogInBrowser | Opening in browser"));

    if( CapsaCore == nullptr || CapsaCore->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "Unable to open Client Log in Browser: CapsaCore Subsystem is invalid." ) );
        return;
    }

    UCapsaCoreSubsystem::OpenBrowser(CapsaCore->LinkWeb);
}

void UCapsaCoreSubsystem::OpenServerLogInBrowser()
{
    UCapsaCoreSubsystem* CapsaCore = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();

    UE_LOG(LogCapsaCore, VeryVerbose, TEXT("UCapsaCoreSubsystem::OpenServerLogInBrowser | Opening in browser"));

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

    //FString LogURL = CapsaCore->GetCapsaLogURL( CapsaCore->CapsaActorComponent->CapsaServerId );
    //UCapsaCoreSubsystem::OpenBrowser( LogURL );
    // FIXME: Reimplement
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
