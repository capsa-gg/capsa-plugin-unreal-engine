// Copyright Companion Group, Ltd. Made available under the MIT license

#include "Settings/CapsaSettings.h"

#include "GameFramework/PlayerState.h"


UCapsaSettings::UCapsaSettings()
	: Protocol( "https" )
	, CapsaServerURL( "" )
	, CapsaEnvironmentKey( "" )
	, LogTickRate( 1.f )
	, MaxTimeBetweenLogFlushes( 10.f )
	, MaxLogLinesBetweenLogFlushes( 100 )
	, bUseCompression( true )
	, bWriteToDisk( true )
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
	return ServerEndpointClientAuth;
}

FString UCapsaSettings::GetServerEndpointClientLogChunk() const
{
	return ServerEndpointClientLogChunk;
}

FString UCapsaSettings::GetServerEndpointClientLogMetadata() const
{
	return ServerEndpointClientLogMetadata;
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

bool UCapsaSettings::GetWriteToDisk() const
{
	return bWriteToDisk;
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
