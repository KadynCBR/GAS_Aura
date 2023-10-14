// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraGameplayTags.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet() {
  OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::ClientEffectApplied);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(
    const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities) {
  for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities) {

    // Loop through abilities and add the startup input tag.
    FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
    if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability)) {
      AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag);
      GiveAbility(AbilitySpec);
    } 
  }
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag) {
  if (!InputTag.IsValid()) return;
  for (auto& AbilitySpec : GetActivatableAbilities()) {
    if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)) {
      AbilitySpecInputPressed(AbilitySpec);
      if(!AbilitySpec.IsActive()) {
        TryActivateAbility(AbilitySpec.Handle);
      }
    }
  }
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag) {
  if (!InputTag.IsValid()) return;
  for (auto& AbilitySpec : GetActivatableAbilities()) {
    if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)) {
      AbilitySpecInputReleased(AbilitySpec);
    }
  }
}

// See H file, but this corresponts to clientEffectApplied.
// Video 108: 9:00 
void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
    const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle) {
  FGameplayTagContainer TagContainer;
  EffectSpec.GetAllAssetTags(TagContainer);
  EffectAssetTags.Broadcast(TagContainer);
}
