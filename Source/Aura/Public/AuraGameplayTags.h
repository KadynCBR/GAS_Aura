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

  // Meta Attributes
  FGameplayTag Attributes_Meta_IncomingXP;

  // Input
  FGameplayTag InputTag_LMB;
  FGameplayTag InputTag_RMB;
  FGameplayTag InputTag_1;
  FGameplayTag InputTag_2;
  FGameplayTag InputTag_3;
  FGameplayTag InputTag_4;
  // not really inputs but used as these can be equipped to slots..
  FGameplayTag InputTag_Passive_1;
  FGameplayTag InputTag_Passive_2;

  // meta
  FGameplayTag Damage;

  // Damage Types
  FGameplayTag Damage_Fire;
  FGameplayTag Damage_Lightning;
  FGameplayTag Damage_Arcane;
  FGameplayTag Damage_Physical;
  FGameplayTag Damage_Poison;

  // Resistances
  FGameplayTag Attributes_Resistance_Fire;
  FGameplayTag Attributes_Resistance_Lightning;
  FGameplayTag Attributes_Resistance_Arcane;
  FGameplayTag Attributes_Resistance_Physical;
  
  // Debuffs
  FGameplayTag Debuff_Burn;
  FGameplayTag Debuff_Stun;
  FGameplayTag Debuff_Arcane;
  FGameplayTag Debuff_Physical;

  // Information tags to be set with magnitude for debuffs.
  // similar to how we're sending damage with meta attributes
  // we're attaching numbers to these tags later
  FGameplayTag Debuff_Chance;
  FGameplayTag Debuff_Damage;
  FGameplayTag Debuff_Duration;
  FGameplayTag Debuff_Frequency;
  
    
    // Ability Tags
  FGameplayTag Abilities_None;
  FGameplayTag Abilities_Attack;
  FGameplayTag Abilities_Summon;

  FGameplayTag Abilities_HitReact;

  // Ability Status Tags
  FGameplayTag Abilities_Status_Locked;
  FGameplayTag Abilities_Status_Eligible;
  FGameplayTag Abilities_Status_Unlocked;
  FGameplayTag Abilities_Status_Equipped;

  // Ability Types
  FGameplayTag Abilities_Type_Offensive; // TODO: Change to Type_Active.
  FGameplayTag Abilities_Type_Passive;
  // ---- used to tag generics, hitreact etc. etc.
  FGameplayTag Abilities_Type_None; 

  // Active Abilities
  FGameplayTag Abilities_Fire_FireBolt;
  FGameplayTag Cooldown_Fire_FireBolt;
  FGameplayTag Abilities_Lightning_Electrocute;

  // Passive Abilities
  FGameplayTag Abilities_Passive_HaloOfProtection;
  FGameplayTag Abilities_Passive_LifeSiphon;
  FGameplayTag Abilities_Passive_ManaSiphon;



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
  TMap<FGameplayTag, FGameplayTag> DamageTypesToDebuffs;

  // StatusEffects
  FGameplayTag Effects_HitReact;

  FGameplayTag Player_Block_InputPressed;
  FGameplayTag Player_Block_InputHeld;
  FGameplayTag Player_Block_InputReleased;
  FGameplayTag Player_Block_CursorTrace;

private:
  static FAuraGameplayTags GameplayTags;
};