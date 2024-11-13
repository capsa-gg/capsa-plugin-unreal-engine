// Copyright Companion Group, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "HttpModule.h"

#include "CapsaCoreSubsystem.generated.h"

// Forward Declarations


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
	virtual void						Initialize( FSubsystemCollectionBase& Collection ) override;
	virtual void						Deinitialize() override;
	// End USubsystem

	/**
	* Whether we have been Authenticated with the Services.
	* 
	* @return bool True if authenticated, otherwise false.
	*/
	bool								IsAuthenticated() const;

	/**
	* Attempts to send the provided Log Buffer to the Capsa Server.
	* 
	* This is performed asynchronously, converted the TArray of BufferedLine's into a single
	* FString Log. If successful, calls RequestSendLog().
	* 
	* @param LogBuffer The Log buffer to parse and send.
	*/
	void								SendLog( TArray<FBufferedLine>& LogBuffer );

protected:

	/**
	* Request a Capsa Auth Token.
	* Builds the response based off details in CapsaSettings. Check and set these in the Editor
	* or Engine.ini.
	* Will call ClientAuthResponse.
	*/
	void								RequestClientAuth();

	/**
	* Callback after a ClientAuth request.
	* 
	* @param Request The FHttpRequestPtr that made the Request.
	* @param Response The FHttpResponsePtr with response information. Payload if successful, error info if not.
	* @param bSuccess Whether the HTTP response was successful (true) or not (false).
	*/
	void								ClientAuthResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess );

	/**
	* Requests to Send a raw Log to the Capsa Server.
	* Internally constructs the URL from the Config settings and uses the Auth token acquired from
	* RequestClientAuth().
	* 
	* @param Log The FString log to attempt to send.
	*/
	void								RequestSendLog( const FString& Log );

	/**
	* Callback after a SendLog request.
	*
	* @param Request The FHttpRequestPtr that made the Request.
	* @param Response The FHttpResponsePtr with response information. Payload if successful, error info if not.
	* @param bSuccess Whether the HTTP response was successful (true) or not (false).
	*/
	void								LogResponse( FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess );

	/**
	* Generic HTTP response processing function.
	* If there is any JSON to gather, will get it out of the Response and return it.
	* 
	* @param LogDetails The Log information (such as calling function mame) to prepend to Log Outputs.
	* @param Request The FHttpRequestPtr that made the Request.
	* @param Response The FHttpResponsePtr with response information. Payload if successful, error info if not.
	* @param bSuccess Whether the HTTP response was successful (true) or not (false).
	* 
	* @return TSharedPtr<FJsonObject> The JSON Object if any found. Can be null.
	*/
	TSharedPtr<FJsonObject>				ProcessResponse( const FString& LogDetails, FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess );

	/**
	* Called after the Map has loaded with the specified World.
	* Perform any specific initialization on Actors that are required when there is a valid World. This is also
	* called when a World changes, such as a new level or joining a server.
	*
	* @param World The UWorld associated with the Map that has loaded.
	*/
	void								OnPostWorldInit( UWorld* World, const UWorld::InitializationValues );

	FDelegateHandle						OnPostWorldInitializationHandle;

private:

	FString								Token;
	FString								LogID;
	FString								LinkWeb;
	FString								Expiry;

};

typedef TFunction<void( const FString& )> FAsyncStringFromBufferCallback;
