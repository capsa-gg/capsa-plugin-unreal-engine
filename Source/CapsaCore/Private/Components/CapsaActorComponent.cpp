// Copyright Companion Group, Ltd. Made available under the MIT license


#include "Components/CapsaActorComponent.h"
#include "CapsaCore.h"
#include "CapsaCoreSubsystem.h"

#include "Net/UnrealNetwork.h"


UCapsaActorComponent::UCapsaActorComponent()
	: CapsaId( "" )
{
	SetIsReplicatedByDefault( true );
	SetAutoActivate( true );
}

void UCapsaActorComponent::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME( UCapsaActorComponent, CapsaId );
	DOREPLIFETIME( UCapsaActorComponent, CapsaServerId );
}

void UCapsaActorComponent::SERVER_SetCapsaId_Implementation( const FString& NewCapsaId )
{
	FString OldCapsaId = CapsaId;
	CapsaId = NewCapsaId;

	// OnRep's aren't called on Servers: call it manually.
	if( GetNetMode() < NM_Client )
	{
		OnCapsaIdUpdated( OldCapsaId );
	}

	UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaActorComponent::SERVER_SetCapsaId_Implementation | OldCapsaId: %s, NewCapsaId: %s"), *OldCapsaId, *NewCapsaId );
}

void UCapsaActorComponent::OnCapsaIdUpdated( FString OldCapsaId )
{
	UCapsaCoreSubsystem* CapsaCoreSubsystem = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();
	if( CapsaCoreSubsystem == nullptr )
	{
		return;
	}

	CapsaCoreSubsystem->RegisterLinkedLogID( CapsaId );

	UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaActorComponent::OnCapsaIdUpdated | CapsaId Updated: %s"), *CapsaId );
}

void UCapsaActorComponent::OnCapsaServerIdUpdated( FString OldCapsaServerId )
{
	UCapsaCoreSubsystem* CapsaCoreSubsystem = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();
	if( CapsaCoreSubsystem == nullptr )
	{
		return;
	}

	CapsaCoreSubsystem->RegisterLinkedLogID( CapsaServerId );

	UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaActorComponent::OnCapsaServerIdUpdated | CapsaServerId Updated: %s"), *CapsaServerId );
}

void UCapsaActorComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaActorComponent::BeginPlay | Begin Play") );

	UCapsaCoreSubsystem* CapsaCoreSubsystem = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();
	if( CapsaCoreSubsystem == nullptr )
	{
		return;
	}

	FString CapsaLogId = CapsaCoreSubsystem->GetLogID();

	if( GetNetMode() < NM_Client )
	{
		CapsaServerId = CapsaLogId;
		OnCapsaServerIdUpdated( TEXT( "" ) );
	}
	
	// Will only reach the server on connections we are authorized to do so.
	SERVER_SetCapsaId( CapsaLogId );
}

void UCapsaActorComponent::EndPlay( EEndPlayReason::Type EndPlayReason )
{
	UE_LOG( LogCapsaCore, Verbose, TEXT("UCapsaActorComponent::BeginPlay | End Play") );

	UCapsaCoreSubsystem* CapsaCoreSubsystem = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();
	if( CapsaCoreSubsystem == nullptr )
	{
		return;
	}

	CapsaCoreSubsystem->UnregisterLinkedLogID( CapsaId );

	Super::EndPlay( EndPlayReason );
}
