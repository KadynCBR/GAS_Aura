// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/AuraInputConfig.h"
#include "InputAction.h"

const UInputAction* UAuraInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const {
  for (const FAuraInputAction& AuraInputAction : AbilityInputActions) {
    if (AuraInputAction.InputAction && AuraInputAction.InputTag.MatchesTagExact(InputTag)) {
      return AuraInputAction.InputAction;
    }
  }
  if (bLogNotFound) {
    UE_LOG(LogTemp, Error, TEXT("Cant find AbilityInputAction for Input Tag %s, on Inputconfig %s."), *InputTag.ToString(), *GetNameSafe(this));
  }
  return nullptr;
}
