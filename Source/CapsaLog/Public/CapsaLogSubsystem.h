// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "CapsaLogSubsystem.generated.h"

#ifndef WITH_CAPSA_LOG_ENABLED
#define WITH_CAPSA_LOG_ENABLED !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#endif

// Forward Declarations
struct FCapsaOutputDevice;

/**
 * UCapsaLogSubsystem is responsible for attaching the FCapsaOutputDevice.
 */
UCLASS()
class CAPSALOG_API UCapsaLogSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:

	// Begin USubsystem
	virtual void						Initialize( FSubsystemCollectionBase& Collection ) override;
	virtual void						Deinitialize() override;
	// End USubsystem

protected:

	/**
	* The collector for Logs that registers itself with Log/Output Redirector
	* and handles compressing/uploading logs to the remote service.
	*/
	TPimplPtr<FCapsaOutputDevice>		CapsaLogOutputDevice;
	
private:


};
