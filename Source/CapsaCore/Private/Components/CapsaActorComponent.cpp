// Copyright Companion Group, Ltd. Made available under the MIT license

#include "Components/CapsaActorComponent.h"

#include "CapsaCore.h"
#include "CapsaCoreSubsystem.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CapsaActorComponent)


FString GetDefaultDescription( bool bIsServer )
{
#if UE_SERVER
	return TEXT("DedicatedServer");
#elif UE_EDITOR
	return TEXT("Editor");
#else
	return bIsServer ? TEXT("ListenServer") : TEXT("Player");
#endif
}

UCapsaActorComponent::UCapsaActorComponent()
	: CapsaServerData( FCapsaSharedData{} )
	, CapsaData( FCapsaSharedData{} )
{
	SetIsReplicatedByDefault( true );
	SetAutoActivate( true );
}

void UCapsaActorComponent::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME( UCapsaActorComponent, CapsaServerData );
}

void UCapsaActorComponent::ServerRegisterLinkedCapsaLog_Implementation( const FCapsaSharedData& ClientCapsaData )
{
	FString OldCapsaId = CapsaData.LogID;
	FString NewCapsaId = ClientCapsaData.LogID;

	CapsaData = ClientCapsaData;

	UE_LOG( LogCapsaCore, Log, TEXT("UCapsaActorComponent::ServerRegisterLinkedCapsaLog_Implementation | OldCapsaId: %s, NewCapsaId: %s"), *OldCapsaId, *NewCapsaId );

	UCapsaCoreSubsystem* CapsaCoreSubsystem = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();
	if( CapsaCoreSubsystem == nullptr )
	{
		UE_LOG( LogCapsaCore, Error, TEXT("UCapsaActorComponent::OnCapsaServerDataUpdated | CapsaCoreSubsystem is nullptr") );
		return;
	}

	UE_LOG( LogCapsaCore, Log, TEXT("UCapsaActorComponent::ServerRegisterLinkedCapsaLog_Implementation | Adding linked log with ID: %s, Description: %s"), *ClientCapsaData.LogID, *ClientCapsaData.Description );
	
	CapsaCoreSubsystem->RegisterLinkedLogID( ClientCapsaData.LogID, ClientCapsaData.Description );
}

void UCapsaActorComponent::OnRep_CapsaServerData()
{
	UCapsaCoreSubsystem* CapsaCoreSubsystem = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();
	if( CapsaCoreSubsystem == nullptr )
	{
		UE_LOG( LogCapsaCore, Error, TEXT("UCapsaActorComponent::OnCapsaServerDataUpdated | CapsaCoreSubsystem is nullptr") );
		return;
	}

	CapsaCoreSubsystem->RegisterLinkedLogID( CapsaServerData.LogID, CapsaServerData.Description );
	CapsaCoreSubsystem->OnServerCapsaDataChangedDynamic.Broadcast( CapsaServerData.LogID, CapsaServerData.LogURL );
	
	UE_LOG( LogCapsaCore, Verbose, TEXT( "UCapsaActorComponent::OnCapsaServerDataUpdated | CapsaServerId Updated: %s" ), *CapsaServerData.ToString() );
}

void UCapsaActorComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaActorComponent::BeginPlay | Start procedure") );

	UCapsaCoreSubsystem* CapsaCoreSubsystem = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();
	if( CapsaCoreSubsystem == nullptr )
	{
		UE_LOG( LogCapsaCore, Error, TEXT("UCapsaActorComponent::BeginPlay | CapsaCoreSubsystem is nullptr") );
		return;
	}

	// Add a callback for whenever the authentication changes
	CapsaCoreSubsystem->OnAuthChanged.AddUObject( this, &UCapsaActorComponent::OnAuthenticationDelegate );

	// Populate the data with the currently present data
	FString CapsaLogId = CapsaCoreSubsystem->GetLogID();
	FString CapsaLogURL = CapsaCoreSubsystem->GetLogURL();

	bool bIsServer = GetIsServer();
	
	CapsaData = FCapsaSharedData(
		CapsaLogId,
		CapsaLogURL,
		GetDefaultDescription( bIsServer )
	);

	UE_LOG( LogCapsaCore, VeryVerbose, TEXT("UCapsaActorComponent::BeginPlay | CapsaData: %s"), *CapsaData.ToString() )

	if( CapsaData.IsEmpty() == true )
	{
		UE_LOG( LogCapsaCore, Warning, TEXT("UCapsaActorComponent::BeginPlay | CapsaData.IsEmpty() == true, not sending data") );
		return;
	}

	// We have authentication, manually call replication logic
	OnAuthenticationDelegate(CapsaLogId, CapsaLogURL);

	if( bIsServer == true )
	{
		// When the server receives authentication data, manually broadcast.
		// Without this, PIE does not correctly display the server UUID.
		CapsaCoreSubsystem->OnServerCapsaDataChangedDynamic.Broadcast( CapsaServerData.LogID, CapsaServerData.LogURL );	
	}
}

void UCapsaActorComponent::EndPlay( EEndPlayReason::Type EndPlayReason )
{
	UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaActorComponent::EndPlay | End Play") );

	UCapsaCoreSubsystem* CapsaCoreSubsystem = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();
	if( CapsaCoreSubsystem == nullptr )
	{
		UE_LOG( LogCapsaCore, Error, TEXT("UCapsaActorComponent::EndPlay | CapsaCoreSubsystem is nullptr") );
		return;
	}

	Super::EndPlay( EndPlayReason );
}

void UCapsaActorComponent::OnAuthenticationDelegate( const FString& LogId, const FString& LogURL )
{
	CapsaData.LogID = LogId;
	CapsaData.LogURL = LogURL;

	UE_LOG( LogCapsaCore, Log, TEXT("UCapsaActorComponent::OnAuthenticationDelegate | After updating authentication data: %s"), *CapsaData.ToString() );
	
	bool bIsServer = GetIsServer();
	
	if( bIsServer )
	{
		CapsaServerData = CapsaData;
		OnRep_CapsaServerData(); // Replicate to clients

		UCapsaCoreSubsystem* CapsaCoreSubsystem = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();
		if( CapsaCoreSubsystem == nullptr )
		{
			UE_LOG( LogCapsaCore, Error, TEXT("UCapsaActorComponent::OnAuthenticationDelegate | CapsaCoreSubsystem is nullptr") );
		} else
		{
			// When the server receives authentication data, manually broadcast.
			// Without this, PIE does not correctly display the server UUID.
			CapsaCoreSubsystem->OnServerCapsaDataChangedDynamic.Broadcast( CapsaServerData.LogID, CapsaServerData.LogURL );	
		}
		
		UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaActorComponent::OnAuthenticationDelegate | OnRep_CapsaServerData called"));
	}
	
	// Will only reach the server on connections we are authorized to do so.
	// This will trigger the server to add a log link to the joined client
	ServerRegisterLinkedCapsaLog( CapsaData );
}

bool UCapsaActorComponent::GetIsServer() const
{
#if !WITH_SERVER_CODE
	return false;
#endif
	
	return GetNetMode() == NM_DedicatedServer || GetNetMode() == NM_ListenServer;
};

