// Copyright Companion Group, Ltd. Made available under the MIT license

#include "Settings/CapsaSettings.h"

#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CapsaSettings)


// NOTE: If the defaults change, this should be updated on the documentation website.
UCapsaSettings::UCapsaSettings()
	: Protocol( "https" )
	, CapsaServerURL( "" )
	, CapsaEnvironmentKey( "" )
	, LogTickRate( 1.f )
#if UE_EDITOR // On editor builds, default to once per 10 minutes
	, MaxTimeBetweenLogFlushes( 600.f )
#else // On non-editor builds, default to 5 minutes
	, MaxTimeBetweenLogFlushes( 300.f )
#endif
	, MaxLogLinesBetweenLogFlushes( 1000 )
	, bUseCompression( true )
	, bWriteToDiskPlain( true )
	, bWriteToDiskCompressed( false )
	, bAutoAddCapsaComponent( true )
	, AutoAddClass( APlayerState::StaticClass() )
{
}

FString UCapsaSettings::GetProtocol() const
{
	return Protocol;
}

FString UCapsaSettings::GetCapsaServerURL() const
{
	return CapsaServerURL;
}

FString UCapsaSettings::GetCapsaEnvironmentKey() const
{
	return CapsaEnvironmentKey;
}

FString UCapsaSettings::GetServerEndpointClientAuth() const
{
	FString Url = GenerateServerBaseURL();
	Url.Append( ServerEndpointClientAuth );
	
	return Url;
}

FString UCapsaSettings::GetServerEndpointClientLogChunk() const
{
	FString Url = GenerateServerBaseURL();
	Url.Append( ServerEndpointClientLogChunk );
	
	return Url;
}

FString UCapsaSettings::GetServerEndpointClientLogMetadata() const
{
	FString Url = GenerateServerBaseURL();
	Url.Append( ServerEndpointClientLogMetadata );
	
	return Url;
}


float UCapsaSettings::GetLogTickRate() const
{
	return LogTickRate;
}

float UCapsaSettings::GetMaxTimeBetweenLogFlushes() const
{
	return MaxTimeBetweenLogFlushes;
}

int32 UCapsaSettings::GetMaxLogLinesBetweenLogFlushes() const
{
	return MaxLogLinesBetweenLogFlushes;
}

bool UCapsaSettings::GetUseCompression() const
{
	return bUseCompression;
}

bool UCapsaSettings::GetWriteToDiskPlain() const
{
	return bWriteToDiskPlain;
}

bool UCapsaSettings::GetWriteToDiskCompressed() const
{
	return bWriteToDiskCompressed;
}

bool UCapsaSettings::GetShouldAutoAddCapsaComponent() const
{
	return bAutoAddCapsaComponent;
}

TSubclassOf<AActor> UCapsaSettings::GetAutoAddClass() const
{
	return AutoAddClass;
}

FString UCapsaSettings::GenerateServerBaseURL() const
{
	// Equivalent to FString::Printf(TEXT( "%s://%s/" )	
	FString ServerBaseUrl = TEXT( "" );
	ServerBaseUrl.Append( GetProtocol() );
	ServerBaseUrl.Append( TEXT( "://" ) );
	ServerBaseUrl.Append( GetCapsaServerURL() );
	ServerBaseUrl.Append( TEXT( "/" ) );
	
	return ServerBaseUrl;
}
