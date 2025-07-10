// Copyright Companion Group, Ltd. Made available under the MIT license

#include "FunctionLibrary/CapsaCoreFunctionLibrary.h"
#include "CapsaCoreSubsystem.h"
#include "Blueprint/BlueprintExceptionInfo.h"
#include "JsonObjectConverter.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(CapsaCoreFunctionLibrary)

FString UCapsaCoreFunctionLibrary::GetPlatformString()
{
	FString Platform = TEXT("Unknown");

#if PLATFORM_WINDOWS
	Platform = TEXT("Win64");
#elif PLATFORM_LINUX
	Platform = TEXT( "Linux" );
#elif PLATFORM_MAC
	Platform = TEXT( "Mac" );
#elif PLATFORM_SWITCH
	Platform = TEXT( "Switch" );
#elif PLATFORM_IOS
	Platform = TEXT( "iOS" );
#elif PLATFORM_ANDROID
	Platform = TEXT( "Android" );
#elif PLATFORM_UNIX
	Platform = TEXT( "Unix" );
#elif PLATFORM_FREEBSD
	Platform = TEXT( "FreeBSD" );
#elif PLATFORM_TVOS
	Platform = TEXT( "TVOS" );
#endif

	return Platform;
}

FString UCapsaCoreFunctionLibrary::GetHostTypeString()
{
	FString HostType = TEXT("Unknown");

#if UE_EDITOR
	HostType = TEXT( "Editor" );
#elif UE_SERVER
	HostType = TEXT("Server");
#elif WITH_SERVER_CODE
	HostType = TEXT( "Game" );
#else
	HostType = TEXT( "Client" );
#endif

	return HostType;
}

FString UCapsaCoreFunctionLibrary::GetLogVerbosityString(ELogVerbosity::Type Verbosity)
{
	switch (Verbosity)
	{
	case ELogVerbosity::Fatal:
		return TEXT("Fatal");
	case ELogVerbosity::Error:
		return TEXT("Error");
	case ELogVerbosity::Warning:
		return TEXT("Warning");
	case ELogVerbosity::Display:
		return TEXT("Display");
	case ELogVerbosity::Log:
		return TEXT("Log");
	case ELogVerbosity::Verbose:
		return TEXT("Verbose");
	case ELogVerbosity::VeryVerbose:
		return TEXT("VeryVerbose");
	default:
		return TEXT("Unknown");
	}
}

DEFINE_FUNCTION(UCapsaCoreFunctionLibrary::execRegisterMetadata)
{
	P_GET_PROPERTY(FStrProperty, MetadataKeyProperty);

	// Move stack parameter index to the first parameter
	Stack.MostRecentProperty = nullptr;
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FProperty>(nullptr);

	FProperty* Property = Stack.MostRecentProperty;
	void* ValuePtr = Stack.MostRecentPropertyAddress;

	TSharedPtr<FJsonValue> JsonValue;

	// Check Supported Types
	if (Property->IsA(FBoolProperty::StaticClass()) ||
		Property->IsA(FIntProperty::StaticClass()) ||
		Property->IsA(FFloatProperty::StaticClass()) ||
		Property->IsA(FStrProperty::StaticClass()))
	{
		ValuePtr = Stack.MostRecentPropertyAddress;
		JsonValue = FJsonObjectConverter::UPropertyToJsonValue(Property, ValuePtr);
	}
	
	P_FINISH;

	if(!JsonValue.IsValid())
	{
		const FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AccessViolation,
			NSLOCTEXT("RegisterMetadata", "CapsaCoreFunctionLibrary_RegisterMetadata_InvalidType", "Only bool, int32, float or string are supported input types")
		);

		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);

		return;
	}

	P_NATIVE_BEGIN;

	UCapsaCoreSubsystem* CapsaCoreSubsystem = GEngine->GetEngineSubsystem<UCapsaCoreSubsystem>();
	if (CapsaCoreSubsystem == nullptr)
	{
		return;
	}

	if (!CapsaCoreSubsystem->IsAuthenticated())
	{
		return;
	}

	CapsaCoreSubsystem->RegisterAdditionalMetadata(MetadataKeyProperty, JsonValue);

	P_NATIVE_END;
}
