// Copyright Cherry Tea Games


#include "AbilitySystem/Abilities/AuraPassiveAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

void UAuraPassiveAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {
  Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
  if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo()))) {
    AuraASC->DeactivatePassiveAbility.AddUObject(this, &UAuraPassiveAbility::ReceiveDeactivate);
  }
}

void UAuraPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag) {
  // We're binding to deactivatepassiveablity on AuraASC, that will broadcast a deactivate to all passives. they check if it's 
  // their tag, if it is, end it.
  if (AbilityTags.HasTagExact(AbilityTag)) {
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
  }
}
  