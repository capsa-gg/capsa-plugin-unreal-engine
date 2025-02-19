// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "Engine/DeveloperSettings.h"

#include "CapsaSettings.generated.h"


UCLASS( Config = Engine, defaultconfig, meta = ( DisplayName = "Capsa Settings" ) )
class CAPSACORE_API UCapsaSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UCapsaSettings();

#pragma region CORE_FUNCTIONS
	/**
	* Get the Protocol used to send Capsa requests.
	* 
	* @return FString The Protocol to use.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Core" )
	FString							GetProtocol() const;

	/**
	* Get the Capsa Server URL.
	* Suffixes are then applied to make actual requests.
	* 
	* @return FString The Capsa Base URL.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Core" )
	FString							GetCapsaServerURL() const;

	/**
	* Get the Capsa Environment Key to authenticate with the Capsa API Server.
	*
	* @return FString The Capsa Environment Key.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Core" )
	FString							GetCapsaEnvironmentKey() const;
#pragma endregion CORE_FUNCTIONS

#pragma region SERVER_ENDPOINTS
	/**
	* Get the full endpoint URL for receiving a log session ID
	*
	* @return FString Endpoint for receiving a log session ID
	*/
	FString							GetServerEndpointClientAuth() const;

	/**
	* Get the full endpoint URL for sending a log chunk
	*
	* @return FString Endpoint for sending a log chunk
	*/
	FString							GetServerEndpointClientLogChunk() const;

	/**
	* Get the full endpoint URL for sending arbitrary log metadata
	*
	* @return FString Endpoint for sending arbitrary log metadata
	*/
	FString							GetServerEndpointClientLogMetadata() const;
#pragma endregion SERVER_ENDPOINTS

#pragma region LOG_FUNCTIONS
	/**
	* Get the Log Tick Rate (in seconds).
	*
	* @return float The Log Tick rate (in seconds).
	*/
	float							GetLogTickRate() const;

	/**
	* Get the maximum time between Log updates/flushes (in seconds).
	*
	* @return float The MaxTimeBetweenLogFlushes (in seconds).
	*/
	float							GetMaxTimeBetweenLogFlushes() const;

	/**
	* Get the maximum number of lines between Log updates/flushes.
	*
	* @return int32 The MaxLogLinesBetweenLogFlushes.
	*/
	int32							GetMaxLogLinesBetweenLogFlushes() const;

	/**
	* Get whether using Compression or not.
	*
	* @return bool Use compression (true) or FString (false).
	*/
	bool							GetUseCompression() const;

	/**
	* Get whether write plain text Log to disk.
	*
	* @return bool Write to disk (true) or not (false).
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Log" )
	bool							GetWriteToDiskPlain() const;

	/**
	* Get whether write compressed Log to disk.
	*
	* @return bool Write to disk (true) or not (false).
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Log" )
	bool							GetWriteToDiskCompressed() const;
#pragma endregion LOG_FUNCTIONS

#pragma region COMPONENT_FUNCTIONS
	/**
	* Get whether the Core Subsystem should auto-create and add a Capsa Component to the 
	* specified class. (see AutoAddClass).
	*
	* @return bool Whether to auto-add the Capsa Component (true) or not (false).
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Component" )
	bool							GetShouldAutoAddCapsaComponent() const;

	/**
	* Get the AActor class which the Capsa Component should be auto-added to.
	*
	* @return TSubclassOf<AActor> The class to auto-add the Capsa Component to.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Component" )
	TSubclassOf<AActor>				GetAutoAddClass() const;

	/**
	* Get the time between Loops to look for the AutoAddClass.
	*
	* @return float The Look for Class Time value.
	*/
	float							GetLookForClassTime() const;

	/**
	* Get the number of times to Loop when looking for AutoAddClass
	*
	* @return int32 The Look for Class Loop Max value.
	*/
	int32							GetLookForClassLoopMax() const;
#pragma endregion COMPONENT_FUNCTIONS

protected:

#pragma region CORE_PROPERTIES
	/**
	* The Protocol to use to access Capsa.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Core", meta = ( DisplayName = "Capsa Server Protocol" ) )
	FString							Protocol;

	/**
	* The Remote URL for where Capsa Service is running.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Core", meta = ( DisplayName = "Capsa Server URL" ) )
	FString							CapsaServerURL;

	/**
	* The Auth Key for making Capsa Service requests.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Core" )
	FString							CapsaEnvironmentKey;
#pragma endregion CORE_PROPERTIES

#pragma region LOG_PROPERTIES
	/**
	* How often (in seconds) should the Log Device check for log lines or time.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log", meta=(Units="Seconds") )
	float							LogTickRate;

	/**
	* How fast (in seconds) to perform the log update and upload check.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log", meta=(Units="Seconds") )
	float							MaxTimeBetweenLogFlushes;

	/**
	* How many lines should be added to the log, before performing the update and upload check.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log" )
	int32							MaxLogLinesBetweenLogFlushes;

	/**
	* Whether we should use Compression (true) or raw FString (false) when sending logs.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log" )
	bool							bUseCompression;

	/**
	* Whether we should write the plain text Log to disk.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log" )
	bool							bWriteToDiskPlain;

	/**
	* Whether we should write the compressed Log to disk.
	* This property is ignored if bUseCompression is set to False.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log" )
	bool							bWriteToDiskCompressed;
#pragma endregion LOG_PROPERTIES

#pragma region COMPONENT_PROPERTIES
	/**
	* Whether the Core Subsystem should auto-create and add a Capsa Component to the specified
	* class.
	* Set to false if you intend to add the component to a replicated actor yourself.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Component" )
	bool							bAutoAddCapsaComponent;

	/**
	* The AActor class which the Capsa Component should be auto-added to.
	* Will only be checked and added if bAutoAddCapsaComponent is true.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Component", Meta=(EditCondition="bAutoAddCapsaComponent" ) )
	TSubclassOf<AActor>				AutoAddClass;
#pragma endregion COMPONENT_PROPERTIES

private:

#pragma region SERVER_ENDPOINTS
	/**
	* Uses the configuration to use generate the Capsa API server base, including trailing slash
	*
	* @return FString Server Base URL that can be used with an Endpoint string to generate the full endpoint URL
	*/
	inline FString					GenerateServerBaseURL() const;

	
	/**
	* Endpoint for receiving a log session ID
	*/
	static inline const FString		ServerEndpointClientAuth = TEXT("v1/client/auth");

	/**
	* Endpoint for sending a log chunk
	*/
	static inline const FString		ServerEndpointClientLogChunk = TEXT("v1/client/log/chunk");

	/**
	* Endpoint for sending arbitraty log metadata
	*/
	static inline const FString		ServerEndpointClientLogMetadata = TEXT("v1/client/log/metadata");
#pragma endregion SERVER_ENDPOINTS

};