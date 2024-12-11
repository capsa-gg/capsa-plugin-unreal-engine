// Copyright Companion Group, Ltd. Made available under the MIT license

#include "CapsaCoreJson.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CapsaCoreJson)

TSharedPtr<FJsonObject> UCapsaCoreJsonHelpers::TMapToJsonObject( const TMap<FString, FString>& Map )
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable( new FJsonObject );

	for( TPair<FString, FString> Item : Map)
	{
		JsonObject->SetStringField( Item.Key, Item.Value );
	}

	return JsonObject;
}

TSharedPtr<FJsonObject> UCapsaCoreJsonHelpers::TMapToJsonObject( const TMap<FString, TSharedPtr<FJsonValue>>& Map )
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable( new FJsonObject );

	for( const TPair<FString, TSharedPtr<FJsonValue>>& Item : Map)
	{
		JsonObject->SetField( Item.Key, Item.Value );
	}
	
	return JsonObject;
}
