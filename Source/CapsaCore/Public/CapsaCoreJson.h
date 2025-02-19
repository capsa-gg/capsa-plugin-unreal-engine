// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "CapsaCoreJson.generated.h"

class UCapsaCoreJsonHelpers
{
	public:

	static TSharedPtr<FJsonObject>	TMapToJsonObject( const TMap<FString, FString>& Map );

	static TSharedPtr<FJsonObject>	TMapToJsonObject( const TMap<FString, TSharedPtr<FJsonValue>>& Map );
};

USTRUCT()
struct FCapsaAuthenticationRequest
{
	GENERATED_BODY()

public:

	FCapsaAuthenticationRequest()
		: Key(TEXT(""))
		, Platform(TEXT(""))
		, Type(TEXT("")) {};
	FCapsaAuthenticationRequest( const FString& InKey, const FString& InPlatform, const FString& InType )
		: Key( InKey )
		, Platform( InPlatform )
		, Type( InType ){};

	UPROPERTY()
	FString							Key;
	
	UPROPERTY()
	FString							Platform;
	
	UPROPERTY()
	FString							Type;
};

USTRUCT()
struct FCapsaAuthenticationResponse
{
	GENERATED_BODY()

	FCapsaAuthenticationResponse()
		: Token(TEXT(""))
		, LogId(TEXT(""))
		, LinkWeb(TEXT(""))
		, Expiry(TEXT("")) {};

public:

	UPROPERTY()
	FString							Token;
	
	UPROPERTY()
	FString							LogId;
	
	UPROPERTY()
	FString							LinkWeb;
	
	UPROPERTY()
	FString							Expiry;
};
