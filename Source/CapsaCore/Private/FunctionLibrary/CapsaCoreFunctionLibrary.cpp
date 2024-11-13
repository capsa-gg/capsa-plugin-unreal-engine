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
