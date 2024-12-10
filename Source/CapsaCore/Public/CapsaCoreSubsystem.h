// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "HttpModule.h"

#include "CapsaCoreSubsystem.generated.h"

// Forward Declarations
class UCapsaActorComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FCapsaCoreOnAuthChangedDynamicDelegate, const FString&, CapsaLogId, const FString&, CapsaLogURL );
DECLARE_MULTICAST_DELEGATE_TwoParams( FCapsaCoreOnAuthChangedDelegate, const FString& /* CapsaLogId */, const FString& /* CapsaLogURL */ );

/**
 * 
 */
UCLASS()
class CAPSACORE_API UCapsaCoreSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:

	UCapsaCoreSubsystem();

	// Begin USubsystem
	virtual void							Initialize( FSubsystemCollectionBase& Collection ) override;
	virtual void							Deinitialize() override;
	// End USubsystem

	/**
	* Delegate that will be called whenever the authentication changes, for example whenever a log session has been created.
	* This delegate should be used in Blueprints.
	*/
	UPROPERTY( BlueprintAssignable )
	FCapsaCoreOnAuthChangedDynamicDelegate	OnAuthChangedDynamic;

	/**
	* Delegate that will be called whenever the authentication changes, for example whenever a log session has been created.
	* This delegate should be used in C++.
	*/
	FCapsaCoreOnAuthChangedDelegate			OnAuthChanged;
	
	/**
	* Whether we have been Authenticated with the Services.
	* 
	* @return bool True if authenticated, otherwise false.
	*/
	bool									IsAuthenticated() const;

	/**
	* Returns the LogID of the currently active connection.
	* 
	* @return FString The LogID.
	*/
	FString									GetLogID() const;

	/**
	* Returns the LogURL of the currently active connection.
	* 
	* @return FString The LogURL.
	*/
	FString									GetLogURL() const;

	/**
	* Attempts to Register the provided Log ID as a Linked Log ID.
	* 
	* @param FString The LinkedLogID to try and register.
	* @param FString The Linked log's description, fe. whether it's a server or client
	* @return bool True if the ID is valid and not already registered. Otherwise false.
	*/
	// TODO: Do not keep this in memory, just store on the server, that's it
	bool									RegisterLinkedLogID( const FString& LinkedLogID, const FString& Description );

	/**
	* Attempts to send the provided Log Buffer to the Capsa Server.
	* 
	* This is performed asynchronously, converted the TArray of BufferedLine's into a single
	* FString Log. If successful, calls RequestSendLog().
	* 
	* @param LogBuffer The Log buffer to parse and send.
	*/
	void									SendLog( TArray<FBufferedLine>& LogBuffer );

	/**
	* Gets the URL for the Client Log and requests the Operating System launch a Browser
	* with the corresponding URL.
	*/
	static void								OpenClientLogInBrowser();

	/**
	* Gets the URL for the Server Log and requests the Operating System launch a Browser
	* with the corresponding URL.
	*/
	static void								OpenServerLogInBrowser();

protected:

	/**
	* Request a Capsa Auth Token.
	* Builds the response based off details in CapsaSettings. Check and set these in the Editor
	* or Engine.ini.
	* Will call ClientAuthResponse.
	*/
	void									RequestClientAuth();

	/**
	* Callback after a ClientAuth request.
	* 
	* @param Request The FHttpRequestPtr that made the Request.
	* @param Response The FHttpResponsePtr with response information. Payload if successful, error info if not.
	* @param bSuccess Whether the HTTP response was successful (true) or not (false).
	*/
	virtual void							ClientAuthResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess );

	/**
	* Requests to Send a raw Log to the Capsa Server.
	* Internally constructs the URL from the Config settings and uses the Auth token acquired from
	* RequestClientAuth().
	* 
	* @param Log The FString log to attempt to send.
	*/
	void									RequestSendLog( const FString& Log );

	/**
	* Requests to Send a Compressed Log to the Capsa Server.
	* Internally constructs the URL from the Config settings and uses the Auth token acquired from
	* RequestClientAuth().
	*
	* @param CompressedLog The TArray<uint8> binary log to attempt to send.
	*/
	void									RequestSendCompressedLog( const TArray<uint8>& CompressedLog );

	/**
	* Callback after a SendLog request.
	*
	* @param Request The FHttpRequestPtr that made the Request.
	* @param Response The FHttpResponsePtr with response information. Payload if successful, error info if not.
	* @param bSuccess Whether the HTTP response was successful (true) or not (false).
	*/
	virtual void							LogResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess );

	/**
	* Generates Capsa supported Metadata and requests to send to the Capsa Server.
	* Internally constructs the URL from the Config settings and uses the LinkedLogIDs TSet acquired from
	* Un/RegisterLinkedLogID.
	*/
	void									RequestSendMetadata();

	/**
	* Callback after a SendMetadata request.
	*
	* @param Request The FHttpRequestPtr that made the Request.
	* @param Response The FHttpResponsePtr with response information. Payload if successful, error info if not.
	* @param bSuccess Whether the HTTP response was successful (true) or not (false).
	*/
	virtual void							MetadataResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess );

	/**
	* Generic HTTP response processing function.
	* If there is any JSON to gather, will get it out of the Response and return it.
	* 
	* @param RequestName The request name (such as calling function name) to prepend to Log Outputs.
	* @param Request The FHttpRequestPtr that made the Request.
	* @param Response The FHttpResponsePtr with response information. Payload if successful, error info if not.
	* @param bSuccess Whether the HTTP response was successful (true) or not (false).
	* 
	* @return TSharedPtr<FJsonObject> The JSON Object if any found. Can be null.
	*/
	virtual TSharedPtr<FJsonObject>			ProcessResponse( const FString& RequestName, FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess );

	/**
	* Called after the Map has loaded with the specified World.
	* Perform any specific initialization on Actors that are required when there is a valid World. This is also
	* called when a World changes, such as a new level or joining a server.
	*
	* @param World The UWorld associated with the Map that has loaded.
	*/
	virtual void							OnPostWorldInit( UWorld* World, const UWorld::InitializationValues );

	/**
	* Called after a Player has been successfully Logged in.
	* Only bound in OnPostWorldInit and only if bAutoAddCapsaComponent is true.
	* 
	* @param GameMode The GameMode the player joined.
	* @param Player The PlayerController for the player that has just joined.
	*/
	virtual void							OnPlayerLoggedIn( AGameModeBase* GameMode, APlayerController* Player );

	/**
	* Called after a Player has been successfully Logged out.
	* Only bound in OnPostWorldInit and only if bAutoAddCapsaComponent is true.
	*
	* @param GameMode The GameMode the player left.
	* @param Player The Controller for the player that has just left.
	*/
	virtual void							OnPlayerLoggedOut( AGameModeBase* GameMode, AController* Controller );

private:

	static void								OpenBrowser( FString URL );

	FDelegateHandle							OnPostWorldInitializationHandle;

	FString									Token;
	FString									LogID;
	FString									LinkWeb;
	FString									Expiry;
	TMap<FString, FString>					LinkedLogIDs;

	TWeakObjectPtr<UCapsaActorComponent>	CapsaActorComponent;

};
