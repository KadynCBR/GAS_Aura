// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
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