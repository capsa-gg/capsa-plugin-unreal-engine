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
	* Get the suffix to append to the Capsa Base URL to generate the full Auth URL.
	*
	* @return FString The Capsa Base URL Auth Suffix.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Core" )
	FString							GetCapsaURLAuthSuffix() const;
#pragma endregion CORE_FUNCTIONS

#pragma region LOG_FUNCTIONS
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
#pragma endregion COMPONENT_FUNCTIONS

protected:

#pragma region CORE_PROPERTIES
	/**
	* The Remote URL for where Capsa Service is running.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Core" )
	FString							CapsaBaseURL;

	/**
	* The Auth Key for making Capsa Service requests.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Core" )
	FString							CapsaAuthKey;

	/**
	* The suffix to add to the CapsaBaseURL to generate the full Auth URL.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Core" )
	FString							CapsaURLAuthSuffix;
#pragma endregion CORE_PROPERTIES

#pragma region LOG_PROPERTIES
	/**
	* The suffix to add to the CapsaBaseURL to generate the full Metadata URL.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log" )
	FString							CapsaURLLogMetadataSuffix;

	/**
	* The suffix to add to the CapsaBaseURL to generate the full Log Cunk URL.
	*/
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Log" )
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
	UPROPERTY( config, EditAnywhere, Category = "Capsa|Component" )
	TSubclassOf<AActor>				AutoAddClass;
#pragma endregion COMPONENT_PROPERTIES

private:

};