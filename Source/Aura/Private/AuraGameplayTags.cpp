// Fill out your copyright notice in the Description page of Project Settings.


#include "AuraGameplayTags.h"
#include "GameplayTagsManager.h"

#define AURATAGADD(l, m) (UGameplayTagsManager::Get().AddNativeGameplayTag(FName(l), FString(m)));

FAuraGameplayTags FAuraGameplayTags::GameplayTags;

void FAuraGameplayTags::InitializeNativeGameplayTags() {
  // Primary Attributes
  GameplayTags.Attributes_Primary_Strength = AURATAGADD("Attributes.Primary.Strength", "Increases physical damage")
  GameplayTags.Attributes_Primary_Intelligence = AURATAGADD("Attributes.Primary.Intelligence", "Increases magic damage and mp regen")
  GameplayTags.Attributes_Primary_Resilience = AURATAGADD("Attributes.Primary.Resilience", "Increase Armor and Armor regen")
  GameplayTags.Attributes_Primary_Vigor = AURATAGADD("Attributes.Primary.Vigor", "Increases HP Regen and Health")
  // Secondary Attributes
  GameplayTags.Attributes_Secondary_Armor = AURATAGADD("Attributes.Secondary.Armor", "reduces damage taken, improves block chance")
  GameplayTags.Attributes_Secondary_ArmorPenetration = AURATAGADD("Attributes.Secondary.ArmorPenetration", "Ignore percent of enemy armor, increase crit chance")
  GameplayTags.Attributes_Secondary_BlockChance = AURATAGADD("Attributes.Secondary.BlockChance", "Chance to cut incoming damage in half")
  GameplayTags.Attributes_Secondary_CriticalHitChance = AURATAGADD("Attributes.Secondary.CriticalHitChance", "Chance to double damage plus crit hit bonus")
  GameplayTags.Attributes_Secondary_CriticalHitDamage = AURATAGADD("Attributes.Secondary.CriticalHitDamage", "Bonus damage added when crit")
  GameplayTags.Attributes_Secondary_CriticalHitResistance = AURATAGADD("Attributes.Secondary.CriticalHitResistance", "reduces critical hit chance of attacking enemies")
  GameplayTags.Attributes_Secondary_HealthRegeneration = AURATAGADD("Attributes.Secondary.HealthRegeneration", "Amount of health regenerated every 1 second")
  GameplayTags.Attributes_Secondary_ManaRegeneration = AURATAGADD("Attributes.Secondary.ManaRegeneration", "Amount of mana regenerated every 1 second")
  GameplayTags.Attributes_Secondary_MaxMana = AURATAGADD("Attributes.Secondary.MaxMana", "Maximum mana obtainable")
  GameplayTags.Attributes_Secondary_MaxHealth = AURATAGADD("Attributes.Secondary.MaxHealth", "Maximum health obtainable")
  // InputTags
  GameplayTags.InputTag_LMB = AURATAGADD("InputTag.LMB","Input Tag for left mouse button");
  GameplayTags.InputTag_RMB = AURATAGADD("InputTag.RMB", "Input Tag for right mouse button");
  GameplayTags.InputTag_1 = AURATAGADD("InputTag.1", "Input Tag for 1 key");
  GameplayTags.InputTag_2 = AURATAGADD("InputTag.2", "Input Tag for 2 key");
  GameplayTags.InputTag_3 = AURATAGADD("InputTag.3", "Input Tag for 3 key");
  GameplayTags.InputTag_4 = AURATAGADD("InputTag.4", "Input Tag for 4 key");

  // MetaTags
  GameplayTags.Damage = AURATAGADD("Damage", "Incoming Damage");

  // Damage types
  GameplayTags.Damage_Fire = AURATAGADD("Damage.Fire", "Fire Damage Type");
  GameplayTags.Damage_Lightning = AURATAGADD("Damage.Lightning", "Lightning Damage Type");
  GameplayTags.Damage_Arcane = AURATAGADD("Damage.Arcane", "Arcane Damage Type");
  GameplayTags.Damage_Physical = AURATAGADD("Damage.Physical", "Physical Damage Type");

  // Resistances
  GameplayTags.Attributes_Resistance_Fire = AURATAGADD("Attributes.Resistance.Fire", "Fire Damage Resistance");
  GameplayTags.Attributes_Resistance_Lightning = AURATAGADD("Attributes.Resistance.Lightning", "Lightning Damage Resistance");
  GameplayTags.Attributes_Resistance_Arcane = AURATAGADD("Attributes.Resistance.Arcane", "Arcane Damage Resistance");
  GameplayTags.Attributes_Resistance_Physical = AURATAGADD("Attributes.Resistance.Physical", "Physical Damage Resistance");

  // Ability Tags
  GameplayTags.Abilities_Attack = AURATAGADD("Abilities.Attack", "Attack Tag");

  
  // Montage Tags
  GameplayTags.Montage_Attack_Weapon = AURATAGADD("Montage.Attack.Weapon", "Montage weapon attack");
  GameplayTags.Montage_Attack_RightHand = AURATAGADD("Montage.Attack.RightHand", "Montage RightHand attack");
  GameplayTags.Montage_Attack_LeftHand = AURATAGADD("Montage.Attack.LeftHand", "Montage LeftHand attack");

  // Map Damage Types to Resistance
  GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Fire, GameplayTags.Attributes_Resistance_Fire);
  GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Lightning, GameplayTags.Attributes_Resistance_Lightning);
  GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Arcane, GameplayTags.Attributes_Resistance_Arcane);
  GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Physical, GameplayTags.Attributes_Resistance_Physical);

  // Status Effects
  GameplayTags.Effects_HitReact = AURATAGADD("Effects.HitReact", "Reacting to being hit")
}
