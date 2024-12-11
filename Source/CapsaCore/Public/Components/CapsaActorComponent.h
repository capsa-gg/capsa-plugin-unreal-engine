// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "CapsaActorComponent.generated.h"


/**
* FCapsaSharedData contains Capsa data that is shared between servers and clients.
 */
USTRUCT( BlueprintType )
struct CAPSACORE_API FCapsaSharedData
{
	GENERATED_BODY()

public:

	FCapsaSharedData() {};
	FCapsaSharedData( const FString& CapsaLogID, const FString& CapsaLogURL, const FString& Description )
		: LogID( CapsaLogID )
		, LogURL( CapsaLogURL )
		, Description( Description ) {};

	/**
	* Contains the UUID to the log session
	*/
	UPROPERTY( BlueprintReadOnly, Category = "Capsa|Log|ActorComponent|SharedData" )
	FString					LogID;

	/**
	* Contains the direct URL to view a log in the Capsa web application.
	* This is necessary as in theory, the server's logs can be stored in a separate instance from the client.
	*/
	UPROPERTY( BlueprintReadOnly, Category = "Capsa|Log|ActorComponent|SharedData" )
	FString					LogURL;

	/**
	* Contains a description of the actor, for example indicating Server vs Client.
	* This can also be used to 
	*/
	UPROPERTY( BlueprintReadOnly, Category = "Capsa|Log|ActorComponent|SharedData" )
	FString					Description;

	/**
	* Returns the contents of the FCapsaSharedData instance as an FString for printing/logging purposes.
	*
	* @return FString string representation of struct data/
	*/
	FString					ToString() const
	{
		return FString::Printf( TEXT( "FCapsaSharedData { LogID: %s, LogURL: %s, Description: %s } " ), *LogID, *LogURL, *Description );
	};

	/**
	* Returns whether the struct should be considered empty.
	*
	* @return bool indicating whether the struct should be considered empty.
	*/
	bool					IsEmpty() const
	{
		return LogID.IsEmpty() || LogURL.IsEmpty();
	}
};


/**
 * UCapsaActorComponent is an actor component that allows zero-setup replication of Capsa data between client and server.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CAPSACORE_API UCapsaActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UCapsaActorComponent();

	virtual void			GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;

	/**
	* Registers the client's Capsa data as a linked log for the server.
	*/
	UFUNCTION( Server, Reliable )
	void					ServerRegisterLinkedCapsaLog( const FCapsaSharedData& ClientCapsaData );
	
	/**
	 * Replication function for CapsaServerData, which adds the server log ID to linked logs.
	 */
	UFUNCTION()
	void					OnRep_CapsaServerData();

	/**
	 * If running on a server, the field contains the server's own data.
	 * If running on a client, the field will contain the data for the connected server.
	 */
	UPROPERTY( ReplicatedUsing = OnRep_CapsaServerData )
	FCapsaSharedData		CapsaServerData;
	

	// TODO: Add way to modify the description for current instance

protected:

	// begin UActorComponent
	virtual void			BeginPlay() override;
	virtual void			EndPlay( EEndPlayReason::Type EndPlayReason ) override;
	// end UActorComponent

	void					OnAuthenticationDelegate( const FString& LogId, const FString& LogURL );
	
	/**
	 * On server: FCapsaSharedData for player that last joined 
	 * On client: client's own FCapsaSharedData.
	 */
	FCapsaSharedData		CapsaData;

	/**
	*  Whether the game instance on which the actor is running should be considered a server or not.
	*
	*  @return bool server status.
    */
	bool					GetIsServer() const;
};
