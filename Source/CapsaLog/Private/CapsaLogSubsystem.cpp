// Copyright Companion Group, Ltd. Made available under the MIT license

#include "CapsaLogSubsystem.h"

#include "CapsaLog.h"
#include "Misc/CapsaOutputDevice.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CapsaLogSubsystem)

void UCapsaLogSubsystem::Initialize( FSubsystemCollectionBase& Collection )
{
	Super::Initialize( Collection );

#if WITH_CAPSA_LOG_ENABLED
	CapsaLogOutputDevice = MakePimpl<FCapsaOutputDevice>();
#endif
}

void UCapsaLogSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

