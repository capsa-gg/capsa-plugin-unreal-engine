// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN( LogCapsaCore, Log, All );

class FCapsaCoreModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void			StartupModule() override;
	virtual void			ShutdownModule() override;
};
