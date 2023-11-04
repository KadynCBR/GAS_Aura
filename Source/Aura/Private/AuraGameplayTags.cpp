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
  GameplayTags.InputTag_Passive_1 = AURATAGADD("InputTag.Passive.1", "Fake Inputtag for Passive 1");
  GameplayTags.InputTag_Passive_2 = AURATAGADD("InputTag.Passive.2", "Fake Inputtag for Passive 1");

  // MetaTags
  GameplayTags.Damage = AURATAGADD("Damage", "Incoming Damage");

  // Damage types
  GameplayTags.Damage_Fire = AURATAGADD("Damage.Fire", "Fire Damage Type");
  GameplayTags.Damage_Lightning = AURATAGADD("Damage.Lightning", "Lightning Damage Type");
  GameplayTags.Damage_Arcane = AURATAGADD("Damage.Arcane", "Arcane Damage Type");
  GameplayTags.Damage_Physical = AURATAGADD("Damage.Physical", "Physical Damage Type");
  GameplayTags.Damage_Poison = AURATAGADD("Damage.Poison", "Poison Damage Type");

  // Resistances
  GameplayTags.Attributes_Resistance_Fire = AURATAGADD("Attributes.Resistance.Fire", "Fire Damage Resistance");
  GameplayTags.Attributes_Resistance_Lightning = AURATAGADD("Attributes.Resistance.Lightning", "Lightning Damage Resistance");
  GameplayTags.Attributes_Resistance_Arcane = AURATAGADD("Attributes.Resistance.Arcane", "Arcane Damage Resistance");
  GameplayTags.Attributes_Resistance_Physical = AURATAGADD("Attributes.Resistance.Physical", "Physical Damage Resistance");

  // MetaAttributes
  GameplayTags.Attributes_Meta_IncomingXP = AURATAGADD("Attributes.Meta.IncomingXP", "Incoming XP");

  // Ability Tags
  GameplayTags.Abilities_None = AURATAGADD("Abilities.None", "No ability. Like nullptr but for abilitytags");
  GameplayTags.Abilities_Attack = AURATAGADD("Abilities.Attack", "Attack Tag");
  GameplayTags.Abilities_Summon = AURATAGADD("Abilities.Summon", "Summoning Tag");

  GameplayTags.Abilities_HitReact = AURATAGADD("Abilities.HitReact", "Hit React Ability"); // I kinda like effects hitreact instead... 

  // Ability Status Tags
  GameplayTags.Abilities_Status_Locked = AURATAGADD("Abilities.Status.Locked", "Ability is locked");
  GameplayTags.Abilities_Status_Eligible = AURATAGADD("Abilities.Status.Eligible", "Ability is eligible for unlock");
  GameplayTags.Abilities_Status_Unlocked = AURATAGADD("Abilities.Status.Unlocked", "Ability is unlocked");
  GameplayTags.Abilities_Status_Equipped = AURATAGADD("Abilities.Status.Equipped", "Ability is equipped");
  // Ability Type Tags
  GameplayTags.Abilities_Type_Offensive = AURATAGADD("Abilities.Type.Offensive", "Offensive Ability types");
  GameplayTags.Abilities_Type_Passive = AURATAGADD("Abilities.Type.Passive", "Passive Ability types");
  GameplayTags.Abilities_Type_None = AURATAGADD("Abilities.Type.None", "Generic ability types, hit react etc.");



  GameplayTags.Abilities_Fire_FireBolt = AURATAGADD("Abilities.Fire.FireBolt", "FireBolt");
  GameplayTags.Cooldown_Fire_FireBolt = AURATAGADD("Cooldown.Fire.FireBolt", "FireBolt Cooldown");
  GameplayTags.Abilities_Lightning_Electrocute = AURATAGADD("Abilities.Lightning.Electrocute", "Electrocute");

  
  // CombatSocket Tags
  GameplayTags.CombatSocket_Weapon = AURATAGADD("CombatSocket.Weapon", "CombatSocket: weapon");
  GameplayTags.CombatSocket_RightHand = AURATAGADD("CombatSocket.RightHand", "CombatSocket: RightHand");
  GameplayTags.CombatSocket_LeftHand = AURATAGADD("CombatSocket.LeftHand", "CombatSocket: LeftHand");
  GameplayTags.CombatSocket_Tail = AURATAGADD("CombatSocket.Tail", "CombatSocket: Tail");

  // Montage Tags
  GameplayTags.Montage_Attack_1 = AURATAGADD("Montage.Attack.1", "Montage attack number 1");
  GameplayTags.Montage_Attack_2 = AURATAGADD("Montage.Attack.2", "Montage attack number 2");
  GameplayTags.Montage_Attack_3 = AURATAGADD("Montage.Attack.3", "Montage attack number 3");
  GameplayTags.Montage_Attack_4 = AURATAGADD("Montage.Attack.4", "Montage attack number 4");
  GameplayTags.Montage_Attack_5 = AURATAGADD("Montage.Attack.5", "Montage attack number 5"); 

  // Map Damage Types to Resistance
  GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Fire, GameplayTags.Attributes_Resistance_Fire);
  GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Lightning, GameplayTags.Attributes_Resistance_Lightning);
  GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Arcane, GameplayTags.Attributes_Resistance_Arcane);
  GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Physical, GameplayTags.Attributes_Resistance_Physical);

  // Status Effects
  GameplayTags.Effects_HitReact = AURATAGADD("Effects.HitReact", "Reacting to being hit")
}
