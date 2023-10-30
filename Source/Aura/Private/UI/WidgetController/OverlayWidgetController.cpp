// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues() {
  Super::BroadcastInitialValues();
  const UAuraAttributeSet* uaa = CastChecked<UAuraAttributeSet>(AttributeSet);
  OnHealthChanged.Broadcast(uaa->GetHealth());
  OnMaxHealthChanged.Broadcast(uaa->GetMaxHealth());
  OnManaChanged.Broadcast(uaa->GetMana());
  OnMaxManaChanged.Broadcast(uaa->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies() {
  Super::BindCallbacksToDependencies();
  const UAuraAttributeSet* uaa = CastChecked<UAuraAttributeSet>(AttributeSet);
  AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    uaa->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnHealthChanged.Broadcast(Data.NewValue);});
  
  AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    uaa->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxHealthChanged.Broadcast(Data.NewValue);});
  
  AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    uaa->GetManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnManaChanged.Broadcast(Data.NewValue);});
  
  AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    uaa->GetMaxManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxManaChanged.Broadcast(Data.NewValue);});
  
  if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)) {
    // If the abilities have been given already, just go ahead and initalize the widget abilities here.
    // otherwise we're binding and once it _does_ happen, it will call that for us.
    if (AuraASC->bStartupAbilitiesGiven) {
      OnInitializeStartupAbilities(AuraASC);
    } else {
      AuraASC->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::OnInitializeStartupAbilities);
    }

    AuraASC->EffectAssetTags.AddLambda(
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

  if (AAuraPlayerState* PS = Cast<AAuraPlayerState>(PlayerState)) {
    PS->OnXPChangedDelegate.AddLambda([this, PS](float XP){ 
      XPDelegate.Broadcast(PS->LevelUpInfo->GetCurrentXPPercent(XP)); 
    });
    PS->OnLevelChangedDelegate.AddLambda([this](float Level) { 
      OnLevelChangedDelegate.Broadcast(Level);                                      
    });
  }
}

void UOverlayWidgetController::OnInitializeStartupAbilities(
    UAuraAbilitySystemComponent* AuraAbilitySystemComponent) {
  if (!AuraAbilitySystemComponent->bStartupAbilitiesGiven) return;
  FForEachAbility BroadcastDelegate;
  BroadcastDelegate.BindLambda([this, AuraAbilitySystemComponent](const FGameplayAbilitySpec& AbilitySpec) {
    // Get the info, to broadcast to widgets
    FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AuraAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
    // make sure info is filled up (input tag)
    Info.InputTag = AuraAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
    AbilityInfoDelegate.Broadcast(Info);
  });
  AuraAbilitySystemComponent->ForEachAbility(BroadcastDelegate);
}
