// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

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
  
  Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->EffectAssetTags.AddLambda(
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
