// Copyright Companion Group, Ltd. Made available under the MIT license

#include "CapsaLog.h"

#define LOCTEXT_NAMESPACE "FCapsaLogModule"

void FCapsaLogModule::StartupModule()
{
}

void FCapsaLogModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FCapsaLogModule, CapsaLog )
DEFINE_LOG_CATEGORY( LogCapsaLog );
