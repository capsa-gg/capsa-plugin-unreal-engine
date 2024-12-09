// Copyright Companion Group, Ltd. Made available under the MIT license

#include "CapsaLogSubsystem.h"

#include "Misc/CapsaOutputDevice.h"


void UCapsaLogSubsystem::Initialize( FSubsystemCollectionBase& Collection )
{
	Super::Initialize( Collection );

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	CapsaLogOutputDevice = MakePimpl<FCapsaOutputDevice>();
#endif
}

void UCapsaLogSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
