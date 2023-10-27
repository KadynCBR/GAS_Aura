// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystemComponent.h"
#include "Aura/Public/AuraGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"

void UAuraProjectileSpell::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData) {
  Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

}
void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride) {
  const bool bIsServer =  GetAvatarActorFromActorInfo()->HasAuthority();
  if (!bIsServer) return;

  const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);
    
  // DESIGN DECISION: Just taking the difference in locations would make the projectile angled.
  // We can set this currently to be parallel to the ground. 
  // If later we wanted to give gravity and make a 'lobbed' spell, we can adjust here as well.
  // Consider: making this an enum and giving options in editor.
  FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
  if (bOverridePitch) {
    Rotation.Pitch = PitchOverride;
  }
  if (!bOverridePitch && bDisablePitch) {
    Rotation.Pitch = 0.f;
  }

  FTransform SpawnTransform;
  SpawnTransform.SetLocation(SocketLocation);
  SpawnTransform.SetRotation(Rotation.Quaternion());

  AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
    ProjectileClass, 
    SpawnTransform,
    GetOwningActorFromActorInfo(),
    Cast<APawn>(GetOwningActorFromActorInfo()),
    ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

  UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
  FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
  // dont need but just showing how it could be done.
  EffectContextHandle.SetAbility(this);
  EffectContextHandle.AddSourceObject(Projectile);

  FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);

  FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

  for (auto& Pair : DamageTypes) {
    const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);

  }

  Projectile->DamageEffectSpecHandle = SpecHandle;

  Projectile->FinishSpawning(SpawnTransform);

}
