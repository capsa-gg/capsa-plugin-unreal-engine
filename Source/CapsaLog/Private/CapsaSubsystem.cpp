// Copyright Companion Group, Ltd. All Rights Reserved.

#include "CapsaSubsystem.h"

#include "Misc/CapsaOutputDevice.h"


void UCapsaSubsystem::Initialize( FSubsystemCollectionBase& Collection )
{
	Super::Initialize( Collection );

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	CapsaLogOutputDevice = MakePimpl<FCapsaOutputDevice>();
#endif
}

void UCapsaSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
