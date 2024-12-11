// Copyright Companion Group, Ltd. Made available under the MIT license

#include "FunctionLibrary/CapsaCoreFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CapsaCoreFunctionLibrary)


FString UCapsaCoreFunctionLibrary::GetPlatformString()
{
	FString Platform = TEXT( "Unknown" );

#if PLATFORM_WINDOWS
	Platform = TEXT( "Win64" );
#elif PLATFORM_LINUX
	Platform = TEXT( "Linux" );
#elif PLATFORM_MAC
	Platform = TEXT( "Mac" );
#elif PLATFORM_SWITCH
	Platform = TEXT( "Switch" );
#elif PLATFORM_IOS
	Platform = TEXT( "iOS" );
#elif PLATFORM_ANDROID
	Platform = TEXT( "Android" );
#elif PLATFORM_UNIX
	Platform = TEXT( "Unix" );
#elif PLATFORM_FREEBSD
	Platform = TEXT( "FreeBSD" );
#elif PLATFORM_TVOS
	Platform = TEXT( "TVOS" );
#endif

	return Platform;
}

FString UCapsaCoreFunctionLibrary::GetHostTypeString()
{
	FString HostType = TEXT( "Unknown" );

#if UE_EDITOR
	HostType = TEXT( "Editor" );
#elif UE_SERVER
	HostType = TEXT( "Server" );
#elif WITH_SERVER_CODE
	HostType = TEXT( "Game" );
#else
	HostType = TEXT( "Client" );
#endif

	return HostType;
}

FString UCapsaCoreFunctionLibrary::GetLogVerbosityString( ELogVerbosity::Type Verbosity )
{
	switch( Verbosity )
	{
	case ELogVerbosity::Fatal:
		return TEXT( "Fatal" );
	case ELogVerbosity::Error:
		return TEXT( "Error" );
	case ELogVerbosity::Warning:
		return TEXT( "Warning" );
	case ELogVerbosity::Display:
		return TEXT( "Display" );
	case ELogVerbosity::Log:
		return TEXT( "Log" );
	case ELogVerbosity::Verbose:
		return TEXT( "Verbose" );
	case ELogVerbosity::VeryVerbose:
		return TEXT( "VeryVerbose" );
	}
	
	return TEXT( "Unknown" );
}
