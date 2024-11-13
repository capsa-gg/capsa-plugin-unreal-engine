// Copyright Companion Group, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN( LogCapsaOTEL, Log, All );

class FCapsaOTELModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void			StartupModule() override;
	virtual void			ShutdownModule() override;
};
