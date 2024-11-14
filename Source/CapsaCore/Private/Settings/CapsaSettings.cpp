// Copyright Companion Group, Ltd. All Rights Reserved.

#include "Settings/CapsaSettings.h"

#include "GameFramework/PlayerState.h"


UCapsaSettings::UCapsaSettings()
	: Protocol( "https://" )
	, APIPrefix( "api." )
	, WebPrefix( "web." )
	, CapsaBaseURL( "" )
	, CapsaAuthKey( "" )
	, CapsaURLAPIPath( "" )
	, CapsaURLAuthSuffix( "auth" )
	, CapsaURLLogSuffix( "log/" )
	, CapsaURLLogMetadataSuffix( "metadata" )
	, CapsaURLLogChunkSuffix( "chunk" )
	, LogTickRate( 1.f )
	, MaxTimeBetweenLogFlushes( 10.f )
	, MaxLogLinesBetweenLogFlushes( 100 )
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

FString UCapsaSettings::GetAPIPrefix() const
{
	return APIPrefix;
}

FString UCapsaSettings::GetWebPrefix() const
{
	return WebPrefix;
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

FString UCapsaSettings::GetCapsaBaseURL() const
{
	return CapsaBaseURL;
}

FString UCapsaSettings::GetCapsaAuthKey() const
{
	return CapsaAuthKey;
}

FString UCapsaSettings::GetCapsaURLAPIPath() const
{
	return CapsaURLAPIPath;
}

FString UCapsaSettings::GetCapsaURLAuthSuffix() const
{
	return CapsaURLAuthSuffix;
}

FString UCapsaSettings::GetCapsaURLLogSuffix() const
{
	return CapsaURLLogSuffix;
}

FString UCapsaSettings::GetCapsaURLLogMetadataSuffix() const
{
	return CapsaURLLogMetadataSuffix;
}

FString UCapsaSettings::GetCapsaURLLogChunkSuffix() const
{
	return CapsaURLLogChunkSuffix;
}
