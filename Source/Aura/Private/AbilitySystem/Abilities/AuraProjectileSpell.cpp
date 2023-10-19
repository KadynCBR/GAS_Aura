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
void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation) {
  const bool bIsServer =  GetAvatarActorFromActorInfo()->HasAuthority();
  if (!bIsServer) return;
  ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
  if (CombatInterface) {
    const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();
    
    // DESIGN DECISION: Just taking the difference in locations would make the projectile angled.
    // We can set this currently to be parallel to the ground. 
    // If later we wanted to give gravity and make a 'lobbed' spell, we can adjust here as well.
    // Consider: making this an enum and giving options in editor.
    FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
    //Rotation.Pitch = 0.f;

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

    const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());

    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Damage, ScaledDamage);

    Projectile->DamageEffectSpecHandle = SpecHandle;

    Projectile->FinishSpawning(SpawnTransform);
  }
}
