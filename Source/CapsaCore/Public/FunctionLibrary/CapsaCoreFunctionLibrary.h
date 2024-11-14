// Copyright Companion Group, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Logging/LogVerbosity.h"

#include "CapsaCoreFunctionLibrary.generated.h"

// Forward Declarations


UCLASS()
class CAPSACORE_API UCapsaCoreFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	* Returns the current Platform as a String.
	* This is in the format Win64, Win32 etc.
	* 
	* @return FString The current Platform as FString.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Core" )
	static FString						GetPlatformString();

	/**
	* Returns whether the build is a Client or Dedicated Server.
	* This is in the format Client or Server.
	*
	* @return FString The current Host Type as FString.
	*/
	UFUNCTION( BlueprintPure, Category = "Capsa|Core" )
	static FString						GetHostTypeString();

	static FString						GetLogVerbosityString( ELogVerbosity::Type Verbosity );
};
