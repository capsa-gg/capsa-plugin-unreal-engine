// Copyright Companion Group, Ltd. All Rights Reserved.


#include "Components/CapsaActorComponent.h"
#include "CapsaCore.h"

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
}

void UCapsaActorComponent::SERVER_SetCapsaId_Implementation( const FString& NewCapsaId )
{
	CapsaId = NewCapsaId;
}

void UCapsaActorComponent::OnCapsaIdUpdated( FString OldCapsaId )
{
	UE_LOG( LogCapsaCore, Log, TEXT( "CapsaId Updated from: %s | to: %s" ), *OldCapsaId, *CapsaId );
}

void UCapsaActorComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG( LogCapsaCore, Log, TEXT( "UCapsaActorComponent::BeginPlay | Success!" ) );
}