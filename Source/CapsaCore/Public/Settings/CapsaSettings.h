// Copyright Companion Group, Ltd. All Rights Reserved.

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
	* Get the API URL Prefix for sending auth/log chunks to Capsa.
	*
	* @return FString The API Prefix.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Core" )
	FString							GetAPIPrefix() const;

	/**
	* Get the Web URL Prefix for generating URLs to access Logs on the hosted Capsa Service.
	*
	* @return FString The Web Prefix.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Core" )
	FString							GetWebPrefix() const;

	/**
	* Get the Capsa Base URL.
	* Suffixes are then applied to make actual requests.
	* 
	* @return FString The Capsa Base URL.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Core" )
	FString							GetCapsaBaseURL() const;

	/**
	* Get the Capsa Authorization Key.
	*
	* @return FString The Capsa Auth Key.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Core" )
	FString							GetCapsaAuthKey() const;

	/**
	* Get the path to append to the Capsa Base URL, that all suffixes must follow to
	* generate full Capsa API URLs.
	*
	* @return FString The Capsa URL API Path.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Core" )
	FString							GetCapsaURLAPIPath() const;

	/**
	* Get the suffix to append to the Capsa Base URL to generate the full Auth URL.
	*
	* @return FString The Capsa Base URL Auth Suffix.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Core" )
	FString							GetCapsaURLAuthSuffix() const;
#pragma endregion CORE_FUNCTIONS

#pragma region LOG_FUNCTIONS
	/**
	* Get the suffix to append to the Capsa Base URL to generate the Log URL.
	* A LogID will be appended after this, provided a successful Auth request has been made.
	*
	* @return FString The Capsa URL Log Suffix.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Log" )
	FString							GetCapsaURLLogSuffix() const;

	/**
	* Get the suffix to append to the Capsa Base URL to generate the Compressed Log URL.
	* A LogID will be appended after this, provided a successful Auth request has been made.
	*
	* @return FString The Capsa URL Compressed Log Suffix.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Log" )
	FString							GetCapsaURLCompressedLogSuffix() const;

	/**
	* Get the suffix to append to the Capsa Base URL to generate the full Log Metadata URL.
	*
	* @return FString The Capsa URL Log Metadata Suffix.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Log" )
	FString							GetCapsaURLLogMetadataSuffix() const;

	/**
	* Get the suffix to append to the Capsa Base URL to generate the full Log Chunk URL.
	*
	* @return FString The Capsa Base URL Log Chunk Suffix.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Log" )
	FString							GetCapsaURLLogChunkSuffix() const;

	/**
	* Get the Log Tick Rate (in seconds).
	*
	* @return float The Log Tick rate (in seconds).
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Log" )
	float							GetLogTickRate() const;

	/**
	* Get the maximum time between Log updates/flushes (in seconds).
	*
	* @return float The MaxTimeBetweenLogFlushes (in seconds).
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Log" )
	float							GetMaxTimeBetweenLogFlushes() const;

	/**
	* Get the maximum number of lines between Log updates/flushes.
	*
	* @return int32 The MaxLogLinesBetweenLogFlushes.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Log" )
	int32							GetMaxLogLinesBetweenLogFlushes() const;

	/**
	* Get whether using Compression or not.
	*
	* @return bool Use compression (true) or FString (false).
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Log" )
	bool							GetUseCompression() const;

	/**
	* Get whether write Log to disk.
	*
	* @return bool Write to disk (true) or not (false).
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Log" )
	bool							GetWriteToDisk() const;
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
	UFUNCTION( BlueprintPure, Category = "Capsa|Component" )
	float							GetLookForClassTime() const;

	/**
	* Get the number of times to Loop when lookin for AutoAddClass
	*
	* @return int32 The Look for Class Loop Max value.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Component" )
	int32							GetLookForClassLoopMax() const;
#pragma endregion COMPONENT_FUNCTIONS

protected:

#pragma region CORE_PROPERTIES
	/**
	* The Protocol to use to access Capsa.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Core" )
	FString							Protocol;

	/**
	* The API Prefix (sub-domain), if needed.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Core", meta = ( DisplayName = "Capsa API Prefix" ) )
	FString							APIPrefix;

	/**
	* The Web Prefix (sub-domain), if needed.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Core" )
	FString							WebPrefix;

	/**
	* The Remote URL for where Capsa Service is running.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Core", meta = ( DisplayName = "Capsa Base URL" ) )
	FString							CapsaBaseURL;

	/**
	* The Auth Key for making Capsa Service requests.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Core" )
	FString							CapsaAuthKey;

	/**
	* The suffix to add to the CapsaBaseURL (and then add suffixes) to generate the full URLs.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Core", meta = ( DisplayName = "Capsa URL API Path" ) )
	FString							CapsaURLAPIPath;

	/**
	* The suffix to add to the CapsaBaseURL to generate the full Auth URL.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Core", meta = ( DisplayName = "Capsa URL Auth Suffix" ) )
	FString							CapsaURLAuthSuffix;
#pragma endregion CORE_PROPERTIES

#pragma region LOG_PROPERTIES
	/**
	* The suffix to add to the CapsaBaseURL to generate the Log Path.
	* A LogID will be appended to this, after a valid Auth request.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log", meta = ( DisplayName = "Capsa URL Log Suffix" ) )
	FString							CapsaURLLogSuffix;

	/**
	* The suffix to add to the CapsaBaseURL to generate the Compressed Log Path.
	* A LogID will be appended to this, after a valid Auth request.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log", meta = ( DisplayName = "Capsa URL Compressed Log Suffix" ) )
	FString							CapsaURLCompressedLogSuffix;

	/**
	* The suffix to add to the CapsaBaseURL to generate the full Metadata URL.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log", meta = ( DisplayName = "Capsa URL Log Metadata Suffix" ) )
	FString							CapsaURLLogMetadataSuffix;

	/**
	* The suffix to add to the CapsaBaseURL to generate the full Log Cunk URL.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log", meta = ( DisplayName = "Capsa URL Log Chunk Suffix" ) )
	FString							CapsaURLLogChunkSuffix;

	/**
	* How often (in seconds) should the Log Device check for log lines or time.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log" )
	float							LogTickRate;

	/**
	* How fast (in seconds) to perform the log update and upload check.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log" )
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
	* Whether we should write the Log to disk.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log" )
	bool							bWriteToDisk;
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

	/**
	* How often, in seconds, should the Timer loop to look for the AutoAddClass.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Component", AdvancedDisplay, Meta = ( EditCondition = "bAutoAddCapsaComponent" ) )
	float							LookForClassTime;

	/**
	* How many times should the Timer look for a matching class.
	* Prevents infinite looping.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Component", AdvancedDisplay, Meta = ( EditCondition = "bAutoAddCapsaComponent" ) )
	int32							LookForClassLoopMax;

#pragma endregion COMPONENT_PROPERTIES

private:

};