// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
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
      AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
      GiveAbility(AbilitySpec);
    } 
  }
  bStartupAbilitiesGiven = true;
  AbilitiesGivenDelegate.Broadcast();
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities) {
  for (TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities) {
    // Loop through abilities and add the startup input tag.
    FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
    GiveAbilityAndActivateOnce(AbilitySpec);
  }
}

void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag) {
  if (!InputTag.IsValid()) return;
  FScopedAbilityListLock ActiveScopeLock(*this);
  for (auto& AbilitySpec : GetActivatableAbilities()) {
    if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)) {
      AbilitySpecInputPressed(AbilitySpec);
      if(!AbilitySpec.IsActive()) {
        InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
      }
    }
  }
}


void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag) {
  if (!InputTag.IsValid()) return;
  FScopedAbilityListLock ActiveScopeLock(*this);
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
  FScopedAbilityListLock ActiveScopeLock(*this);
  for (auto& AbilitySpec : GetActivatableAbilities()) {
    if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive()) {
      AbilitySpecInputReleased(AbilitySpec);
      InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
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

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec) {
  for (FGameplayTag StatusTag : AbilitySpec.DynamicAbilityTags) {
    if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status")))) {
      return StatusTag;
    }
  }
  return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag) {
  if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag)) {
    return GetStatusFromSpec(*Spec);
  }
  return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetSlotFromAbilityTag(const FGameplayTag& AbilityTag) {
  if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag)) {
    return GetInputTagFromSpec(*Spec);
  }
  return FGameplayTag();  
}

bool UAuraAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot) {
  FScopedAbilityListLock ActiveScopeLock(*this);
  for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities()) {
    if (AbilityHasSlot(AbilitySpec, Slot)) {
      return false;
    }
  }
  return true;
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecWithSlot(const FGameplayTag& Slot) {
  FScopedAbilityListLock ActiveScopeLock(*this);
  for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities()) {
    if (AbilitySpec.DynamicAbilityTags.HasTagExact(Slot)) {
      return &AbilitySpec;
    }
  }
  return nullptr;
}

bool UAuraAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& Spec) const {
  const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
  const FGameplayTag AbilityTag = GetAbilityTagFromSpec(Spec);
  const FAuraAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
  const FGameplayTag AbilityType = Info.AbilityType;
  return AbilityType.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Type_Passive);
}

void UAuraAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot) {
  ClearSlot(&Spec);
  Spec.DynamicAbilityTags.AddTag(Slot);
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag) {
  // Lock ability list for this scope while we're iterating.
  FScopedAbilityListLock ActiveScopeLock(*this);
  for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities()) {
    for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags) {
      if (Tag.MatchesTag(AbilityTag)) {
        return &AbilitySpec;
      } 
    }
  }
  return nullptr;
}

void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag) {
  // might be on client or on server. make sure we have points before sending rpc and stuff.
  if (GetAvatarActor()->Implements<UPlayerInterface>()) {
    if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0) {
      ServerUpgradeAttribute(AttributeTag);
    } 
  }
}

// Giving abilities that we become eligible for based on level.
void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level) {
  UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
  for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation) {
    if (Level < Info.LevelRequirement) continue;
    if (!Info.AbilityTag.IsValid()) continue;
    // if nullptr, we dont have it yet or it doesn't exist yet for us.
    if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr) {
      FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
      AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
      GiveAbility(AbilitySpec);
      // markabilityspecdirty forces replication immediately
      MarkAbilitySpecDirty(AbilitySpec);
      ClientUpdateAbilityStatus(Info.AbilityTag, FAuraGameplayTags::Get().Abilities_Status_Eligible, 1);
    }
  }
}

void UAuraAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Slot) {
  // Check this ability even exists in our abilities
  if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag)) {
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec);
    const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);
    // only can equip if its already equipped, or unlocked.
    const bool bStatusValid = Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.Abilities_Status_Unlocked;
    if (bStatusValid) {
      if (!SlotIsEmpty(Slot)) {
        // Ability in this slot already deactivate and clear its slot.
        FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(Slot);
        if (SpecWithSlot) {
          // If same ability, return early.
          if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot))) { 
            ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
            return;
          }
          // If passive, broadcast a deactivation, our passives are bound to this delegate on creation
          // using their set ability tag.
          if (IsPassiveAbility(*SpecWithSlot)) {
            DeactivatePassiveAbility.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
          }
          ClearSlot(SpecWithSlot);
        }
      }
      if (!AbilityHasAnySlot(*AbilitySpec)) { // Ability doesn't yet have a slot. (its not active)
        if (IsPassiveAbility(*AbilitySpec)) {
          TryActivateAbility(AbilitySpec->Handle);
        }
      }
      AssignSlotToAbility(*AbilitySpec, Slot);
      MarkAbilitySpecDirty(*AbilitySpec);
    }
    ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
  }
}

void UAuraAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) {
  AbilityEquipped.Broadcast(AbilityTag, Status, Slot, PreviousSlot);
}

bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(
    const FGameplayTag& AbilityTag, FString& OutDescription,
    FString& OutNextDescription) {
  if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag)) {
    if (UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability)) {
      OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);
      OutNextDescription = AuraAbility->GetNextLevelDescription(AbilitySpec->Level+1);
      return true;
    }
  }
  UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
  if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_None)) {
    OutDescription = FString();
  } else {
    OutDescription = UAuraGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
  }
  OutNextDescription = FString();
  return false;
}

void UAuraAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec) {
  const FGameplayTag& Slot = GetInputTagFromSpec(*Spec);
  Spec->DynamicAbilityTags.RemoveTag(Slot);
}

void UAuraAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& Slot) {
  FScopedAbilityListLock ActiveScopeLock(*this);
  for (FGameplayAbilitySpec& Spec: GetActivatableAbilities()) {
    if (AbilityHasSlot(Spec, Slot)) {
      ClearSlot(&Spec);
    }
  }
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot) {
  return Spec.DynamicAbilityTags.HasTagExact(Slot);
}

bool UAuraAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& Spec) {
  return Spec.DynamicAbilityTags.HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag) {
  if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag)) {
    const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
    FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
    if (GetAvatarActor()->Implements<UPlayerInterface>()) {
      IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
    }
    if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible)) { 
      // eligible -> Unlocked
      AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Eligible);
      AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);
      Status = GameplayTags.Abilities_Status_Unlocked;
    } else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked)) {
      // Upgrading
      AbilitySpec->Level += 1; // level up the ability if it's not active
    }
    ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
    MarkAbilitySpecDirty(*AbilitySpec);
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
    AbilitiesGivenDelegate.Broadcast();
  }
}

void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel) {
  AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

// See H file, but this corresponts to clientEffectApplied.
// Video 108: 9:00 
void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
    const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle) {
  FGameplayTagContainer TagContainer;
  EffectSpec.GetAllAssetTags(TagContainer);
  EffectAssetTags.Broadcast(TagContainer);
}
