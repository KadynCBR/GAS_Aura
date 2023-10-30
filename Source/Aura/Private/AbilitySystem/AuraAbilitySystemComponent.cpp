// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Interaction/PlayerInterface.h"
#include "Aura/AuraLogChannels.h"
#include "AuraGameplayTags.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet() {
  OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::ClientEffectApplied);
}

// Only done on server
void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities) {
  for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities) {
    // Loop through abilities and add the startup input tag.
    FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
    if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability)) {
      AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag);
      GiveAbility(AbilitySpec);
    } 
  }
  bStartupAbilitiesGiven = true;
  AbilitiesGivenDelegate.Broadcast(this);
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities) {
  for (TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities) {
    // Loop through abilities and add the startup input tag.
    FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
    GiveAbilityAndActivateOnce(AbilitySpec);
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

void UAuraAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate) {
  // Lock the ability list incase it happens to change during this process.
  // this is why we handle this foreach here instead of outside.
  FScopedAbilityListLock ActiveScopeLock(*this);
  for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities()) {
    if (!Delegate.ExecuteIfBound(AbilitySpec)) {
      UE_LOG(LogAura, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
    }
  }
}

FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec) {
  // Loop through the ability tags on the spec's ability.
  // ASSUMING!!! only one is marked "Abilities" then we'll return that tag.
  // this should be the case, as our abilities are activated via this tag.
  if (AbilitySpec.Ability) {
    for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags) {
      if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities")))) {
        return Tag;
      }
    }
  }
  return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec) {
  // Just like our ability tags, we should only have one input tag at a time.
  for (FGameplayTag Tag : AbilitySpec.DynamicAbilityTags) {
    if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag")))) {
      return Tag;
    }
  }
  return FGameplayTag();
}

void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag) {
  // might be on client or on server. make sure we have points before sending rpc and stuff.
  if (GetAvatarActor()->Implements<UPlayerInterface>()) {
    if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0) {
      ServerUpgradeAttribute(AttributeTag);
    } 
  }
}

void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag) {
  FGameplayEventData Payload;
  Payload.EventTag = AttributeTag;
  Payload.EventMagnitude = 1.f;

  UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

  if (GetAvatarActor()->Implements<UPlayerInterface>()) {
    IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
  }
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities() {
  Super::OnRep_ActivateAbilities();
  // Only do this the first time. On client this should still be false until we set it here.
  if (!bStartupAbilitiesGiven) {
    bStartupAbilitiesGiven = true;
    AbilitiesGivenDelegate.Broadcast(this);
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
