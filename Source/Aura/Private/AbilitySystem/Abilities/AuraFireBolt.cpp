// Copyright Cherry Tea Games


#include "AbilitySystem/Abilities/AuraFireBolt.h"
#include "Aura/Public/AuraGameplayTags.h"
#include "Actor/AuraProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

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

void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride, AActor* HomingTarget) {
  const bool bIsServer =  GetAvatarActorFromActorInfo()->HasAuthority();
  if (!bIsServer) return;
  const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);
  FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
  if (bOverridePitch) { Rotation.Pitch = PitchOverride; }
  else if (!bOverridePitch && bDisablePitch) { Rotation.Pitch = 0.f; }
  const FVector Forward = Rotation.Vector();
  int32 EffectiveNumProjectiles = FMath::Min(NumProjectiles, GetAbilityLevel());
  TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, EffectiveNumProjectiles);
  for (const FRotator& Rot : Rotations) {
    FTransform SpawnTransform;
    SpawnTransform.SetLocation(SocketLocation);
    SpawnTransform.SetRotation(Rot.Quaternion());
    
    AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
      ProjectileClass, 
      SpawnTransform,
      GetOwningActorFromActorInfo(),
      Cast<APawn>(GetOwningActorFromActorInfo()),
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
    Projectile->FinishSpawning(SpawnTransform);
    if (HomingTarget && HomingTarget->Implements<UCombatInterface>()) {
      Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
    } else {
      // if click on something other than enemy.
      Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
      Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
      Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
    }
    Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelMin, HomingAccelMax);
    Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectiles;
  }
}
