// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "CapsaCoreJson.generated.h"

class UCapsaCoreJsonHelpers
{
public:
	static TSharedPtr<FJsonObject> TMapToJsonObject(const TMap<FString, FString>& Map);

	static TSharedPtr<FJsonObject> TMapToJsonObject(const TMap<FString, TSharedPtr<FJsonValue>>& Map);
};

/// Request to authenticate with the Capsa server
USTRUCT()
struct FCapsaAuthenticationRequest
{
	GENERATED_BODY()

public:
	FCapsaAuthenticationRequest() :
		Key(TEXT("")),
		Platform(TEXT("")),
		Type(TEXT(""))
	{
	};

	FCapsaAuthenticationRequest(const FString& InKey, const FString& InPlatform, const FString& InType) :
		Key(InKey),
		Platform(InPlatform),
		Type(InType)
	{
	};

	UPROPERTY()
	FString Key; ///< Environment key to identify which Environment/Title the log should be stored under

	UPROPERTY()
	FString Platform; ///< Indicates the platform the game is running on

	UPROPERTY()
	FString Type; ///< Indicates whether the log is from Editor, Client, Game or Server
};

/// Returned when successfully authenticating with the Capsa server
USTRUCT()
struct FCapsaAuthenticationResponse
{
	GENERATED_BODY()

	FCapsaAuthenticationResponse() :
		Token(TEXT("")),
		LogId(TEXT("")),
		LinkWeb(TEXT("")),
		Expiry(TEXT(""))
	{
	};

public:
	UPROPERTY()
	FString Token; ///< JWT that should be included when sending log chunks or metadata

	UPROPERTY()
	FString LogId; ///< UUID that is used to identify the current log

	UPROPERTY()
	FString LinkWeb; ///< Direct link to view the log in the Capsa web app

	UPROPERTY()
	FString Expiry; ///< Timestamp of when the Token will expire, should be longer than any expected game session
};
