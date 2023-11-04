// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues() {
  Super::BroadcastInitialValues();
  OnHealthChanged.Broadcast(GetAuraAS()->GetHealth());
  OnMaxHealthChanged.Broadcast(GetAuraAS()->GetMaxHealth());
  OnManaChanged.Broadcast(GetAuraAS()->GetMana());
  OnMaxManaChanged.Broadcast(GetAuraAS()->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies() {
  Super::BindCallbacksToDependencies();
  AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    GetAuraAS()->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnHealthChanged.Broadcast(Data.NewValue);});
  
  AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    GetAuraAS()->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxHealthChanged.Broadcast(Data.NewValue);});
  
  AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    GetAuraAS()->GetManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnManaChanged.Broadcast(Data.NewValue);});
  
  AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    GetAuraAS()->GetMaxManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxManaChanged.Broadcast(Data.NewValue);});
  
  if (GetAuraASC()) {
    GetAuraASC()->AbilityEquipped.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);
    // If the abilities have been given already, just go ahead and initalize the widget abilities here.
    // otherwise we're binding and once it _does_ happen, it will call that for us.
    if (GetAuraASC()->bStartupAbilitiesGiven) {
      BroadcastAbilityInfo();
    } else {
      GetAuraASC()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
    }

    GetAuraASC()->EffectAssetTags.AddLambda(
      [this](const FGameplayTagContainer& AssetTags) {
         for (const FGameplayTag& Tag : AssetTags) {
            FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
            if (Tag.MatchesTag(MessageTag)) {
              FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
              MessageWidgetRowDelegate.Broadcast(*Row);
            }
         }
      }
    );
  }

  if (AAuraPlayerState* PS = GetAuraPS()) {
    PS->OnXPChangedDelegate.AddLambda([this](float XP){ 
      XPDelegate.Broadcast(AuraPlayerState->LevelUpInfo->GetCurrentXPPercent(XP)); 
    });
    PS->OnLevelChangedDelegate.AddLambda([this](float Level) { 
      OnLevelChangedDelegate.Broadcast(Level);                                      
    });
  }
}

void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) const {
  // Similar to spellmenuOnAbilityEquipped, we're just doing it for the overlay hud.
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
}
