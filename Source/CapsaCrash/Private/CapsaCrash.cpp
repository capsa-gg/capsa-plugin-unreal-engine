// Copyright Companion Group, Ltd. Made available under the MIT license

#include "CapsaCrash.h"

#define LOCTEXT_NAMESPACE "FCapsaCrashModule"

void FCapsaCrashModule::StartupModule()
{
}

void FCapsaCrashModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FCapsaCrashModule, CapsaCrash )
DEFINE_LOG_CATEGORY( LogCapsaCrash );
