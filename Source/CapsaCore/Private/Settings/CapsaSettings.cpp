// Copyright Companion Group, Ltd. All Rights Reserved.

#include "Settings/CapsaSettings.h"

#include "GameFramework/PlayerState.h"


UCapsaSettings::UCapsaSettings()
	: CapsaBaseURL( "" )
	, CapsaAuthKey( "" )
	, CapsaURLAuthSuffix( "" )
	, CapsaURLLogMetadataSuffix( "" )
	, CapsaURLLogChunkSuffix( "" )
	, LogTickRate( 1.f )
	, MaxTimeBetweenLogFlushes( 10.f )
	, MaxLogLinesBetweenLogFlushes( 100 )
	, bAutoAddCapsaComponent( true )
	, AutoAddClass( APlayerState::StaticClass() )
{
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

FString UCapsaSettings::GetCapsaBaseURL() const
{
	return CapsaBaseURL;
}

FString UCapsaSettings::GetCapsaAuthKey() const
{
	return CapsaAuthKey;
}

FString UCapsaSettings::GetCapsaURLAuthSuffix() const
{
	return CapsaURLAuthSuffix;
}

FString UCapsaSettings::GetCapsaURLLogMetadataSuffix() const
{
	return CapsaURLLogMetadataSuffix;
}

FString UCapsaSettings::GetCapsaURLLogChunkSuffix() const
{
	return CapsaURLLogChunkSuffix;
}
