// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Player/AuraPlayerState.h"
#include "AuraGameplayTags.h"

void UAttributeMenuWidgetController::BroadcastInitialValues() {
  UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
  check(AttributeInfo);

  for (auto& Pair : AS->TagsToAttributes) {
    BroadcastAttributeInfo(Pair.Key, Pair.Value());
  }
  if (AAuraPlayerState* PS = Cast<AAuraPlayerState>(PlayerState)) {
    AttributePointsChangedDelegate.Broadcast(PS->GetAttributePoints());
    SpellPointsChangedDelegate.Broadcast(PS->GetSpellPoints());
  }
}

void UAttributeMenuWidgetController::BindCallbacksToDependencies() {
  UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
  check(AttributeInfo);
  for (auto& Pair : AS->TagsToAttributes) {
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
      [this, Pair](const FOnAttributeChangeData& Data){
        BroadcastAttributeInfo(Pair.Key, Pair.Value());
      }
    );
  }

  if (AAuraPlayerState* PS = Cast<AAuraPlayerState>(PlayerState)) {
    PS->OnAttributePointsChangedDelegate.AddLambda([this](int32 Points){
      AttributePointsChangedDelegate.Broadcast(Points);
    });
    PS->OnSpellPointsChangedDelegate.AddLambda([this](int32 Points){
      SpellPointsChangedDelegate.Broadcast(Points);
    });
  }
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag) {
  UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
  AuraASC->UpgradeAttribute(AttributeTag);
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(
    const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const {
  FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
  Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
  AttributeInfoDelegate.Broadcast(Info);
}
