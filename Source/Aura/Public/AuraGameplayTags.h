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

  // Resistances
  FGameplayTag Attributes_Resistance_Fire;
  FGameplayTag Attributes_Resistance_Lightning;
  FGameplayTag Attributes_Resistance_Arcane;
  FGameplayTag Attributes_Resistance_Physical;

  // Input
  FGameplayTag InputTag_LMB;
  FGameplayTag InputTag_RMB;
  FGameplayTag InputTag_1;
  FGameplayTag InputTag_2;
  FGameplayTag InputTag_3;
  FGameplayTag InputTag_4;
  // meta
  FGameplayTag Damage;

  // Damage Types
  FGameplayTag Damage_Fire;
  FGameplayTag Damage_Lightning;
  FGameplayTag Damage_Arcane;
  FGameplayTag Damage_Physical;

  // Ability Tags
  FGameplayTag Abilities_Attack;
  FGameplayTag Abilities_Summon;

  // Combat Socket
  FGameplayTag CombatSocket_Weapon;
  FGameplayTag CombatSocket_RightHand;
  FGameplayTag CombatSocket_LeftHand;
  FGameplayTag CombatSocket_Tail; // maybe we can use this as body?

  // Montage Tags (Used to tie specific montages and attack events.
  // Should be treated as generic tags to be used, if enemies have more than N attacks, just add more ig
  FGameplayTag Montage_Attack_1;
  FGameplayTag Montage_Attack_2;
  FGameplayTag Montage_Attack_3;
  FGameplayTag Montage_Attack_4;
  FGameplayTag Montage_Attack_5;

  TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;

  // StatusEffects
  FGameplayTag Effects_HitReact;

private:
  static FAuraGameplayTags GameplayTags;
};