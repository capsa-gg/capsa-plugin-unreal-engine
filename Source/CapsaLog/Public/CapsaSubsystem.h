// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "CapsaSubsystem.generated.h"

// Forward Declarations
struct FCapsaOutputDevice;

/**
 * 
 */
UCLASS()
class CAPSALOG_API UCapsaSubsystem : public UEngineSubsystem
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
