// Copyright Companion Group, Ltd. Made available under the MIT license

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
	/// Returns the current Platform as a String. This is in the format Win64, Win32 etc.
	/// @return FString The current Platform as FString.
	UFUNCTION(BlueprintPure, Category = "Capsa|Core")
	static FString GetPlatformString();

	/// Returns whether the build is a Client or Dedicated Server. This is in the format Client or Server.
	/// @return FString The current Host Type as FString.
	UFUNCTION(BlueprintPure, Category = "Capsa|Core")
	static FString GetHostTypeString();

	/// Converts the ELogVerbosity::Type to a Capsa-compatible verbosity string
	/// @return FString ELogVerbosity::Type value as a string
	static FString GetLogVerbosityString(ELogVerbosity::Type Verbosity);

	/// <summary>
	/// Attempts to register the Metadata with the provided Key and Value pair.
	/// Currently only bool, int32, float and FString are supported.
	/// </summary>
	/// <param name="MetadataKey">The FString for the Key relating to this metadata.</param>
	/// <param name="InputValue">The target value. In Blueprint, this is a wildcard.</param>
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(CustomStructureParam="InputValue"))
	static void RegisterMetadata(const FString& MetadataKey, const int& InputValue);
	DECLARE_FUNCTION(execRegisterMetadata);

};
