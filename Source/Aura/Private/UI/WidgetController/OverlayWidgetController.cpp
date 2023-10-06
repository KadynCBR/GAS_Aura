// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"

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
    uaa->GetHealthAttribute()).AddUObject(this, &UOverlayWidgetController::HealthChanged);
  AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    uaa->GetMaxHealthAttribute()).AddUObject(this, &UOverlayWidgetController::MaxHealthChanged);
  AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    uaa->GetManaAttribute()).AddUObject(this, &UOverlayWidgetController::ManaChanged);
  AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    uaa->GetMaxManaAttribute()).AddUObject(this, &UOverlayWidgetController::MaxManaChanged);

}

void UOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const {
  OnHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const {
  OnMaxHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::ManaChanged(const FOnAttributeChangeData& Data) const {
  OnManaChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxManaChanged(const FOnAttributeChangeData& Data) const {
  OnMaxManaChanged.Broadcast(Data.NewValue);
}
