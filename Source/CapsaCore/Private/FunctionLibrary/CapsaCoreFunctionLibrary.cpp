// Copyright Companion Group, Ltd. All Rights Reserved.

#include "FunctionLibrary/CapsaCoreFunctionLibrary.h"




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
#endif

	return Platform;
}

FString UCapsaCoreFunctionLibrary::GetHostTypeString()
{
	FString HostType = TEXT( "Client" );

#if UE_SERVER
	HostType = TEXT( "Server" );
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
