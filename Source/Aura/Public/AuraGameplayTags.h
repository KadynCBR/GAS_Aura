// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * AuraGameplayTags
 * Singleton containing native Gameplay Tags
 */

struct FAuraGameplayTags {
public:
  static const FAuraGameplayTags& Get() {return GameplayTags;}
  static void InitializeNativeGameplayTags();

  // Primary Attrs
  FGameplayTag Attributes_Primary_Strength;
  FGameplayTag Attributes_Primary_Intelligence;
  FGameplayTag Attributes_Primary_Resilience;
  FGameplayTag Attributes_Primary_Vigor;
  // Secondary Attrs
  FGameplayTag Attributes_Secondary_Armor;
  FGameplayTag Attributes_Secondary_ArmorPenetration;
  FGameplayTag Attributes_Secondary_BlockChance;
  FGameplayTag Attributes_Secondary_CriticalHitChance;
  FGameplayTag Attributes_Secondary_CriticalHitDamage;
  FGameplayTag Attributes_Secondary_CriticalHitResistance;
  FGameplayTag Attributes_Secondary_HealthRegeneration;
  FGameplayTag Attributes_Secondary_ManaRegeneration;
  FGameplayTag Attributes_Secondary_MaxMana;
  FGameplayTag Attributes_Secondary_MaxHealth;
  // Input
  FGameplayTag InputTag_LMB;
  FGameplayTag InputTag_RMB;
  FGameplayTag InputTag_1;
  FGameplayTag InputTag_2;
  FGameplayTag InputTag_3;
  FGameplayTag InputTag_4;
  // meta
  FGameplayTag Damage;

  // StatusEffects
  FGameplayTag Effects_HitReact;

private:
  static FAuraGameplayTags GameplayTags;
};