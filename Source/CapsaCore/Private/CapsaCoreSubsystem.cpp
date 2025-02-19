// Copyright Companion Group, Ltd. Made available under the MIT license

#include "CapsaCoreSubsystem.h"

#include "CapsaCore.h"
#include "CapsaCoreAsync.h"
#include "CapsaCoreJson.h"
#include "JsonObjectConverter.h"
#include "FunctionLibrary/CapsaCoreFunctionLibrary.h"
#include "Settings/CapsaSettings.h"

#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CapsaCoreSubsystem)


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

void UCapsaCoreSubsystem::RegisterMetadataString( const FString& Key, const FString& Value )
{
    TSharedPtr<FJsonValueString> JsonValue = MakeShared<FJsonValueString>( Value );
    RegisterAdditionalMetadata( Key, JsonValue );
    UE_LOG( LogCapsaCore, VeryVerbose, TEXT("UCapsaCoreSubsystem::RegisterMetadata | Registered metadata with key: %s, value :%s"), *Key, *Value );
}

FCapsaSharedData UCapsaCoreSubsystem::GetServerCapsaData() const
{
    if( CapsaActorComponent.IsValid() == false )
    {
        UE_LOG( LogCapsaCore, Verbose, TEXT( "UCapsaCoreSubsystem::GetServerCapsaData | No valid reference to CapsaActorComponent" ) );
        return FCapsaSharedData{};
    }

    return CapsaActorComponent->CapsaServerData;
}

bool UCapsaCoreSubsystem::IsAuthenticated() const
{
    return ( Token.IsEmpty() == false ) && ( LogID.IsEmpty() == false );
}

FString UCapsaCoreSubsystem::GetLogID() const
{
    return LogID;
}

FString UCapsaCoreSubsystem::GetLogURL() const
{
    return LinkWeb;
}

bool UCapsaCoreSubsystem::RegisterLinkedLogID( const FString& LinkedLogID, const FString& Description )
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

    UE_LOG( LogCapsaCore, Log, TEXT( "UCapsaCoreSubsystem::RegisterLinkedLogID | Registering LinkedLogID: %s" ), *LinkedLogID );
    
    LinkedLogIDs.Add( LinkedLogID, Description );
    RequestSendMetadata();
    return true;
}

void UCapsaCoreSubsystem::RegisterAdditionalMetadata( const FString& Key, const TSharedPtr<FJsonValue>& Value )
{
    AdditionalMetadata.Add( Key, Value );
    UE_LOG( LogCapsaCore, VeryVerbose, TEXT( "UCapsaCoreSubsystem::RegisterAdditionalMetadata | Added metadata with key %s" ), *Key );
    RequestSendMetadata();
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
    UE_LOG( LogCapsaCore, Verbose, TEXT( "UCapsaCoreSubsystem::RequestClientAuth | Starting client authentication" ) );
    
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

    FCapsaAuthenticationRequest AuthenticationRequest = FCapsaAuthenticationRequest(
        CapsaSettings->GetCapsaEnvironmentKey(),
        UCapsaCoreFunctionLibrary::GetPlatformString(),
        UCapsaCoreFunctionLibrary::GetHostTypeString()
    );
    
    FString AuthContent;
    if( FJsonObjectConverter::UStructToJsonObjectString( AuthenticationRequest, AuthContent ) == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::RequestClientAuth | FJsonObjectConverter::UStructToJsonObjectString has failed" ) );
    }

    FHttpRequestRef ClientAuthRequest = FHttpModule::Get().CreateRequest();
    ClientAuthRequest->SetURL( CapsaSettings->GetServerEndpointClientAuth() );
    ClientAuthRequest->SetVerb( "POST" );
    ClientAuthRequest->SetHeader( "Content-Type", "application/json" );
    ClientAuthRequest->SetContentAsString( AuthContent );
    ClientAuthRequest->OnProcessRequestComplete().BindUObject( this, &UCapsaCoreSubsystem::ClientAuthResponse );
    ClientAuthRequest->ProcessRequest();

    UE_LOG( LogCapsaCore, Log, TEXT("UCapsaCoreSubsystem::RequestClientAuth | Authentication request sent") );
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

    FHttpRequestRef LogRequest = FHttpModule::Get().CreateRequest();
    LogRequest->SetURL( CapsaSettings->GetServerEndpointClientLogChunk() );
    LogRequest->SetVerb( "POST" );
    LogRequest->SetHeader( "Authorization", GetAuthHeader() );
    LogRequest->SetHeader( "Content-Type", "text/plain" );
    LogRequest->SetContentAsString( Log );
    LogRequest->OnProcessRequestComplete().BindUObject( this, &UCapsaCoreSubsystem::LogResponse );
    LogRequest->ProcessRequest();

    UE_LOG( LogCapsaCore, VeryVerbose, TEXT("UCapsaCoreSubsystem::RequestSendLog | Log sent") );
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

    FHttpRequestRef LogRequest = FHttpModule::Get().CreateRequest();
    LogRequest->SetURL( CapsaSettings->GetServerEndpointClientLogChunk() );
    LogRequest->SetVerb( "POST" );
    LogRequest->SetHeader( "Authorization", GetAuthHeader() );
    LogRequest->SetHeader( "Content-Type", "application/zlib" );
    LogRequest->SetContent( CompressedLog );
    LogRequest->OnProcessRequestComplete().BindUObject( this, &UCapsaCoreSubsystem::LogResponse );
    LogRequest->ProcessRequest();

    UE_LOG( LogCapsaCore, VeryVerbose, TEXT("UCapsaCoreSubsystem::RequestSendCompressedLog | Compressed log sent") );
}

void UCapsaCoreSubsystem::RequestSendMetadata()
{
    UE_LOG (LogCapsaCore, Verbose, TEXT("UCapsaCoreSubsystem::RequestSendMetadata | Storing metadata") );

    const UCapsaSettings* CapsaSettings = GetDefault<UCapsaSettings>();
    if( CapsaSettings == nullptr || CapsaSettings->IsValidLowLevelFast() == false )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "UCapsaCoreSubsystem::RequestSendMetadata | Failed to load CapsaSettings" ) );
        return;
    }
    
    TSharedPtr<FJsonObject> JsonObject = MakeShareable( new FJsonObject );
    JsonObject->SetObjectField( TEXT( "linkedLogs" ), UCapsaCoreJsonHelpers::TMapToJsonObject(LinkedLogIDs) );
    JsonObject->SetObjectField( TEXT( "additionalMetadata" ), UCapsaCoreJsonHelpers::TMapToJsonObject(AdditionalMetadata) );

    FString MetadataContent;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create( &MetadataContent, 0 );
    FJsonSerializer::Serialize( JsonObject.ToSharedRef(), Writer );

    FHttpRequestRef LogRequest = FHttpModule::Get().CreateRequest();
    LogRequest->SetURL( CapsaSettings->GetServerEndpointClientLogMetadata() );
    LogRequest->SetVerb( "POST" );
    LogRequest->SetHeader( "Authorization", GetAuthHeader() );
    LogRequest->AppendToHeader( "Content-Type", "application/json" );
    LogRequest->SetContentAsString( MetadataContent );
    LogRequest->OnProcessRequestComplete().BindUObject( this, &UCapsaCoreSubsystem::LogResponse );
    LogRequest->ProcessRequest();

    UE_LOG (LogCapsaCore, VeryVerbose, TEXT("UCapsaCoreSubsystem::RequestSendMetadata | Metadata sent") );
}

void UCapsaCoreSubsystem::ClientAuthResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess )
{
    UE_LOG( LogCapsaCore, Log, TEXT("UCapsaCoreSubsystem::ClientAuthResponse | Authentication resonse received sent") );

    TSharedPtr<FJsonObject> JsonObject = ProcessResponse( TEXT( "UCapsaCoreSubsystem::ClientAuthResponse" ), Request, Response, bSuccess );
    if( JsonObject == nullptr || JsonObject.IsValid() == false )
    {
        UE_LOG( LogCapsaCore, Warning, TEXT( "UCapsaCoreSubsystem::ClientAuthResponse | Invalid JSON object" ) );
        return;
    }
    
    FCapsaAuthenticationResponse AuthenticationResponse;
    if ( FJsonObjectConverter::JsonObjectToUStruct( JsonObject.ToSharedRef(), &AuthenticationResponse ) == false )
    {
        UE_LOG( LogCapsaCore, Warning, TEXT( "UCapsaCoreSubsystem::ClientAuthResponse | FJsonObjectConverter::JsonObjectToUStruc failed" ) );
        return;
    };

    // Set the authentication data if the current data is empty
    if( Token.IsEmpty() || LogID.IsEmpty() || LinkWeb.IsEmpty() )
    {
        UE_LOG( LogCapsaCore, Verbose, TEXT( "UCapsaCoreSubsystem::ClientAuthResponse | Authentication info not present, setting values" ) );
        Token = AuthenticationResponse.Token;
        LogID = AuthenticationResponse.LogId;
        LinkWeb = AuthenticationResponse.LinkWeb;
        Expiry = AuthenticationResponse.Expiry;
        UE_LOG( LogCapsaCore, Log, TEXT( "UCapsaCoreSubsystem::ClientAuthResponse | Capsa ID: %s | CapsaLogURL: %s" ), *LogID, *LinkWeb);
    } else
    {
        UE_LOG( LogCapsaCore, Log, TEXT( "UCapsaCoreSubsystem::ClientAuthResponse | Ignoring AuthenticationResponse (CapsaID: %s), as the authentication is already present" ), *LogID );
    }
    
    // Broadcast auth changed regardless whether it has changed or not
    OnAuthChanged.Broadcast( LogID, LinkWeb );
    OnAuthChangedDynamic.Broadcast( LogID, LinkWeb );
}

void UCapsaCoreSubsystem::LogResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess )
{
    UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaCoreSubsystem::LogResponse | Log chunk stored") );

    ProcessResponse( TEXT( "UCapsaCoreSubsystem::LogResponse" ), Request, Response, bSuccess );
}

void UCapsaCoreSubsystem::MetadataResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess )
{
    UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaCoreSubsystem::MetadataResponse | Metadata stored") );

    // Remove metadata on success
    if( bSuccess == true && Response->GetResponseCode() < 299 )
    {
        UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaCoreSubsystem::MetadataResponse | Clearing metadata.") );
        LinkedLogIDs.Empty();
        AdditionalMetadata.Empty();
    }

    ProcessResponse( TEXT( "UCapsaCoreSubsystem::MetadataResponse" ), Request, Response, bSuccess );
}

TSharedPtr<FJsonObject> UCapsaCoreSubsystem::ProcessResponse( const FString& RequestName, FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess )
{
    // Exit early if request failed
    if( bSuccess == false )
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
        UE_LOG( LogCapsaCore, Error, TEXT( "Unable to open Server Log in Browser: CapsaActorComponent is not valid" ) );
        return;
    }

    if( CapsaCore->CapsaActorComponent->CapsaServerData.LogURL.IsEmpty() == true )
    {
        UE_LOG( LogCapsaCore, Error, TEXT( "Unable to open Server Log in Browser: CapsaActorComponent->CapsaServerData.LogURL is empty" ) );
        return;
    }

    FString LogURL = CapsaCore->CapsaActorComponent->CapsaServerData.LogURL;
    UCapsaCoreSubsystem::OpenBrowser( LogURL );
}

void UCapsaCoreSubsystem::OpenBrowser( const FString& URL )
{
    FPlatformProcess::LaunchURL( *URL, nullptr, nullptr );
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
