// Copyright Companion Group, Ltd. All Rights Reserved.

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

	UFUNCTION( Server, Reliable )
	void					SERVER_SetCapsaId( const FString& NewCapsaId );

	UFUNCTION()
	void					OnCapsaIdUpdated( FString OldCapsaId );

	UPROPERTY( ReplicatedUsing = OnCapsaIdUpdated )
	FString					CapsaId;

protected:

	virtual void			BeginPlay() override;
		
};
