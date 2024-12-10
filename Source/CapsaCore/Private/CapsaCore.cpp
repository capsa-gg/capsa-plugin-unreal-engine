// Copyright Companion Group, Ltd. Made available under the MIT license

#include "CapsaCore.h"


#define LOCTEXT_NAMESPACE "FCapsaCoreModule"

void FCapsaCoreModule::StartupModule()
{
}

void FCapsaCoreModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FCapsaCoreModule, CapsaCore )
DEFINE_LOG_CATEGORY( LogCapsaCore );
