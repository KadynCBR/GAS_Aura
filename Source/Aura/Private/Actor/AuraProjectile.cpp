

#include "Actor/AuraProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Aura/Aura.h"

AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
  bReplicates = true;
  Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
  SetRootComponent(Sphere);
  Sphere->SetCollisionObjectType(ECC_Projectile);
  Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
  Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
  Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

  ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
  ProjectileMovement->InitialSpeed = 550.f;
  ProjectileMovement->MaxSpeed = 550.f;
  ProjectileMovement->ProjectileGravityScale = 0.f;

}

void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
  SetLifeSpan(LifeSpan);
  SetReplicateMovement(true);
  Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);
  LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
}

void AAuraProjectile::Destroyed() {
  if (LoopingSoundComponent) {
    LoopingSoundComponent->Stop();
    LoopingSoundComponent->DestroyComponent();
  }
  if (!bHit && !HasAuthority()) { OnHit(); }
  Super::Destroyed();
}

void AAuraProjectile::OnHit() {
  UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(),FRotator::ZeroRotator);
  UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
  if (LoopingSoundComponent) {
    LoopingSoundComponent->Stop();
    LoopingSoundComponent->DestroyComponent();
  }
  bHit = true;
}

void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
                                      AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp,
                                      int32 OtherBodyIndex, bool bFromSweep,
                                      const FHitResult& SweepResult) {
  if (!IsValidOverlap(OtherActor)) return;
  if (!bHit) { OnHit(); }
  if (HasAuthority()) {
    // Finish Damage Effect Params, we now know who the target is (our overlapactor) so now we set it.
    if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor)) {
      const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
      FRotator Rotation = GetActorRotation();
      Rotation.Pitch = 45.f;
      const FVector KnockbackDirection = Rotation.Vector();
      const FVector KnockbackVector = KnockbackDirection * DamageEffectParams.KnockbackMagnitude;
      DamageEffectParams.DeathImpulse = DeathImpulse;
      DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
      DamageEffectParams.KnockbackVector = KnockbackVector;
      UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
    }
    Destroy();
  } else { bHit = true; }
}

bool AAuraProjectile::IsValidOverlap(AActor* OtherActor) {
  if (DamageEffectParams.SourceAbilitySystemComponent == nullptr) return false;
  AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();
  if (SourceAvatarActor == OtherActor) return false;
  if (!UAuraAbilitySystemLibrary::IsNotFriend(SourceAvatarActor, OtherActor)) return false;
  return true;
}
