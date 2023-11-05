// Copyright Cherry Tea Games

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "DebuffNiagaraComponent.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UDebuffNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()
 public:
	UDebuffNiagaraComponent();

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DebuffTag;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount);


	UFUNCTION()
	void OnOwnerDeath(AActor* DeadActor);
};