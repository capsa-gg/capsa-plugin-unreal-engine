// Copyright Companion Group, Ltd. Made available under the MIT license

#include "Settings/CapsaSettings.h"

#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CapsaSettings)


UCapsaSettings::UCapsaSettings()
	: Protocol( "https" )
	, CapsaServerURL( "" )
	, CapsaEnvironmentKey( "" )
	, LogTickRate( 1.f )
	, MaxTimeBetweenLogFlushes( 10.f )
	, MaxLogLinesBetweenLogFlushes( 100 )
	, bUseCompression( true )
	, bWriteToDiskPlain( true )
	, bWriteToDiskCompressed( false )
	, bAutoAddCapsaComponent( true )
	, AutoAddClass( APlayerState::StaticClass() )
	, LookForClassTime( 1.f )
	, LookForClassLoopMax( 5 )
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

float UCapsaSettings::GetLookForClassTime() const
{
	return LookForClassTime;
}

int32 UCapsaSettings::GetLookForClassLoopMax() const
{
	return LookForClassLoopMax;
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
