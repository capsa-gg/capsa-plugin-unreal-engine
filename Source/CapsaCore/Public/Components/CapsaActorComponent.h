// Copyright Companion Group, Ltd. Made available under the MIT license

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "CapsaActorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CAPSACORE_API UCapsaActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UCapsaActorComponent();

	virtual void			GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;

	/**
	* Request setting the CapsaId for this connection.
	*/
	UFUNCTION( Server, Reliable )
	void					SERVER_SetCapsaId( const FString& NewCapsaId );

	UFUNCTION()
	void					OnCapsaIdUpdated( FString OldCapsaId );

	UFUNCTION()
	void					OnCapsaServerIdUpdated( FString OldCapsaServerId );

	UPROPERTY( ReplicatedUsing = OnCapsaIdUpdated )
	FString					CapsaId;

	UPROPERTY( ReplicatedUsing = OnCapsaServerIdUpdated )
	FString					CapsaServerId;

protected:

	virtual void			BeginPlay() override;
	virtual void			EndPlay( EEndPlayReason::Type EndPlayReason ) override;
		
};
