// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "CapsaCoreJson.generated.h"

// TODO: Can this be removed?
USTRUCT()
struct FCapsaMetadataRequest
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TMap<FString, FString> LinkedLogs;
	
	UPROPERTY()
	TMap<FString, FString> AdditionalMetadata;
};
