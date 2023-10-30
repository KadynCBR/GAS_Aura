// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerState.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Net/UnrealNetwork.h"

AAuraPlayerState::AAuraPlayerState() {
  NetUpdateFrequency = 100.f; // how fast values are updated across network
  AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
  AbilitySystemComponent->SetIsReplicated(true);
  AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
  
  AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
}

void AAuraPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(AAuraPlayerState, Level);
  DOREPLIFETIME(AAuraPlayerState, XP);
  DOREPLIFETIME(AAuraPlayerState, AttributePoints);
  DOREPLIFETIME(AAuraPlayerState, SpellPoints);
}

UAbilitySystemComponent* AAuraPlayerState::GetAbilitySystemComponent() const {
  return AbilitySystemComponent;
}

void AAuraPlayerState::SetXP(int32 InXP) {
  XP = InXP;
  OnXPChangedDelegate.Broadcast(XP); // serverside
}

void AAuraPlayerState::SetLevel(int32 InLevel) {
  Level = InLevel;
  OnLevelChangedDelegate.Broadcast(Level);
}

void AAuraPlayerState::SetAttributePoints(int32 InPoints) {
  AttributePoints = InPoints;
  OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void AAuraPlayerState::SetSpellPoints(int32 InPoints) {
  SpellPoints = InPoints;
  OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

void AAuraPlayerState::AddToXP(int32 InXP) {
  SetXP(XP + InXP); 
}

void AAuraPlayerState::AddToLevel(int32 InLevel) {
  SetLevel(InLevel + Level);
}

void AAuraPlayerState::AddToAttributePoints(int32 InPoints) {
  SetAttributePoints(AttributePoints+InPoints);
}

void AAuraPlayerState::AddToSpellPoints(int32 InPoints) {
  SetSpellPoints(SpellPoints+InPoints);
}

void AAuraPlayerState::OnRep_Level(int32 OldLevel) {
  OnLevelChangedDelegate.Broadcast(Level);
}

void AAuraPlayerState::OnRep_XP(int32 OldXP) {
  OnXPChangedDelegate.Broadcast(XP); // Clientside for UI and local elements.
}

void AAuraPlayerState::OnRep_AttributePoints(int32 OldAttributePoints) {
  OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void AAuraPlayerState::OnRep_SpellPoints(int32 OldSpellPoints) {
  OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}
