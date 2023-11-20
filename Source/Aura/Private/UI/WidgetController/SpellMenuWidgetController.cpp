// Copyright Cherry Tea Games


#include "UI/WidgetController/SpellMenuWidgetController.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerState.h"

void USpellMenuWidgetController::BroadcastInitialValues() {
  BroadcastAbilityInfo();
  SpellPointsChangedDelegate.Broadcast(GetAuraPS()->GetSpellPoints());
}

void USpellMenuWidgetController::BindCallbacksToDependencies() {

  // Catch the delegate from AuraASC, bind and then send forward to spellmenuUI
  GetAuraASC()->AbilityStatusChanged.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 NewLevel){
    
    // If the ability we just had a status change on is our selected ability, run button selected routine to update ui.
    if (SelectedAbility.Ability.MatchesTagExact(AbilityTag)) {
      SelectedAbility.Status = StatusTag;
      bool bPointButtonEnabled = false;
      bool bEquipButtonEnabled = false;
      ShouldEnableButtons(StatusTag, CurrentSpellPoints, bPointButtonEnabled, bEquipButtonEnabled);
      FString Description;
      FString NextLevelDescription;
      GetAuraASC()->GetDescriptionsByAbilityTag(AbilityTag, Description, NextLevelDescription);
      ButtonSelectedDelegate.Broadcast(bPointButtonEnabled, bEquipButtonEnabled, Description, NextLevelDescription);
    }
    if (AbilityInfo) {
      FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
      Info.StatusTag = StatusTag;
      AbilityInfoDelegate.Broadcast(Info);
    }
  });

  GetAuraASC()->AbilityEquipped.AddUObject(this, &USpellMenuWidgetController::OnAbilityEquipped);

  GetAuraPS()->OnSpellPointsChangedDelegate.AddLambda([this](int32 Points){
    SpellPointsChangedDelegate.Broadcast(Points);
    CurrentSpellPoints = Points;
    bool bPointButtonEnabled = false;
    bool bEquipButtonEnabled = false;
    ShouldEnableButtons(SelectedAbility.Status, CurrentSpellPoints, bPointButtonEnabled, bEquipButtonEnabled);
    FString Description;
    FString NextLevelDescription;
    GetAuraASC()->GetDescriptionsByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription);
    ButtonSelectedDelegate.Broadcast(bPointButtonEnabled, bEquipButtonEnabled, Description, NextLevelDescription);
  });
}

void USpellMenuWidgetController::OnButtonSelected(const FGameplayTag& AbilityTag) {
  if (bWaitingForEquipSelection) {
    FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType;
    StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
    bWaitingForEquipSelection = false;
  }
  const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
  const int32 SpellPoints = GetAuraPS()->GetSpellPoints();
  FGameplayTag AbilityStatus;
  
  const bool bTagValid = AbilityTag.IsValid();
  const bool bTagNone = AbilityTag.MatchesTag(GameplayTags.Abilities_None);
  FGameplayAbilitySpec* AbilitySpec = GetAuraASC()->GetSpecFromAbilityTag(AbilityTag);
  const bool bSpecValid = AbilitySpec != nullptr;
  if (!bTagValid || bTagNone || !bSpecValid) {
    // If we get ther, we're dealing with an abilitytag for an ability we dont have yet, or an ability that doesn't exist.
    AbilityStatus = GameplayTags.Abilities_Status_Locked;
  } else {
    AbilityStatus = GetAuraASC()->GetStatusFromSpec(*AbilitySpec);
  }
  SelectedAbility.Ability = AbilityTag;
  SelectedAbility.Status = AbilityStatus;
  bool bPointButtonEnabled = false;
  bool bEquipButtonEnabled = false;
  ShouldEnableButtons(AbilityStatus, SpellPoints, bPointButtonEnabled, bEquipButtonEnabled);
  FString Description;
  FString NextLevelDescription;
  GetAuraASC()->GetDescriptionsByAbilityTag(AbilityTag, Description, NextLevelDescription);
  ButtonSelectedDelegate.Broadcast(bPointButtonEnabled, bEquipButtonEnabled, Description, NextLevelDescription);
}

void USpellMenuWidgetController::SpendPointButtonPressed() {
  if (GetAuraASC()) {
    GetAuraASC()->ServerSpendSpellPoint(SelectedAbility.Ability);
  }
}

void USpellMenuWidgetController::GlobeDeselect() {
  if (bWaitingForEquipSelection) {
    FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
    StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
    bWaitingForEquipSelection = false;
  }
  SelectedAbility.Ability = FAuraGameplayTags::Get().Abilities_None;
  SelectedAbility.Ability = FAuraGameplayTags::Get().Abilities_Status_Locked;
  ButtonSelectedDelegate.Broadcast(false, false, FString(), FString());
}

void USpellMenuWidgetController::EquipButtonPressed() {
  const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
  WaitForEquipDelegate.Broadcast(AbilityType);
  bWaitingForEquipSelection = true;

  const FGameplayTag SelectedStatus = GetAuraASC()->GetStatusFromAbilityTag(SelectedAbility.Ability);
  // If our Ability is equipped, save the slot its equipped to to clear later.
  if (SelectedStatus.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped)) {
    SelectedSlot = GetAuraASC()->GetSlotFromAbilityTag(SelectedAbility.Ability);
  }
}

void USpellMenuWidgetController::SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType) {
  if (!bWaitingForEquipSelection) return;
  // check selected ability against slot type, dont equip passive to active slot.
  const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
  if (!SelectedAbilityType.MatchesTagExact(AbilityType)) return;
  GetAuraASC()->ServerEquipAbility(SelectedAbility.Ability, SlotTag);
}

void USpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) {
  bWaitingForEquipSelection = false;
  const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

  // Clear out previous stuff.
  FAuraAbilityInfo LastSlotInfo;
  LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
  LastSlotInfo.InputTag = PreviousSlot;
  LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
  // Broadcast empty info if PreviousSlot is a valid slot. only if equipping an already equipped spell
  AbilityInfoDelegate.Broadcast(LastSlotInfo);

  // Add new stuff.
  FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
  Info.StatusTag = StatusTag;
  Info.InputTag = Slot;
  AbilityInfoDelegate.Broadcast(Info);
  
  // Stop animations
  StopWaitingForEquipDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);
  SpellGlobeReassigned.Broadcast(AbilityTag);
  GlobeDeselect();
}

void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bPointButtonEnabled, bool& bEquipButtonEnabled) {
  const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
  bEquipButtonEnabled = (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) ||
                         AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked));
  bPointButtonEnabled = (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) ||
                         AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible) ||
                         AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked));
  bPointButtonEnabled = SpellPoints > 0 && bPointButtonEnabled;

}
