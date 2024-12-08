// Copyright Companion Group, Ltd. Made available under the MIT license

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
