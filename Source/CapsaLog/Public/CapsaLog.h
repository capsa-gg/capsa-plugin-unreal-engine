// Copyright Companion Group, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN( LogCapsa, Log, All );

class FCapsaLogModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void			StartupModule() override;
	virtual void			ShutdownModule() override;
};
