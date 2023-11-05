// Copyright Cherry Tea Games


#include "AbilitySystem/Abilities/AuraFireBolt.h"
#include "Aura/Public/AuraGameplayTags.h"

FString UAuraFireBolt::GetDescription(int32 Level) { 
  const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
  const float ManaCost = GetManaCost(Level) * -1;
  const float Cooldown = GetCooldown(Level);
  if (Level == 1) {
    return FString::Printf(TEXT(
      // title
      "<Title> FIRE BOLT</>\n\n"
      // details
      "<Small>Level: </><Level>%d</>\n"
      "<Small>ManaCost: </><ManaCost>%.1f</>\n"
      "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
      
      // Description
      "<Default>Launched a bolt of fire, "
      "exploding on impact and dealing:</> <Damage>%d</><Default> "
      "fire damage with a chance to burn</>\n\n"
    ), 
      Level,
      ManaCost,
      Cooldown,
      ScaledDamage 
    );
  } else {
    return FString::Printf(TEXT(
      // Title
      "<Title> FIRE BOLT</>\n\n"
      // Details
      "<Small>Level: </><Level>%d</>\n"
      "<Small>ManaCost: </><ManaCost>%.1f</>\n"
      "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
      // Description
      "<Default>Launches %d bolts of fire, "
      "exploding on impact and dealing:</> <Damage>%d</><Default> "
      "fire damage with a chance to burn</>\n\n"),
      Level,
      ManaCost,
      Cooldown,
      ScaledDamage, 
      FMath::Min(Level, NumProjectiles)
    );
  }
}

FString UAuraFireBolt::GetNextLevelDescription(int32 Level) {
  const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
  return FString::Printf(TEXT(
    "<Title> Next Level:</>\n\n"
    "<Default>Launches %d bolts of fire, exploding on impact and dealing:</> "
    "<Damage>%d</><Default> fire damage with a chance to burn</>\n\n<Small>Level: </><Level>%d</>"
  ),FMath::Min(Level, NumProjectiles), ScaledDamage, Level);
}
