

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
  Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);
  LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
}

void AAuraProjectile::Destroyed() {
  if (!bHit && !HasAuthority()) { OnHit(); }
  Super::Destroyed();
}

void AAuraProjectile::OnHit() {
  UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(),FRotator::ZeroRotator);
  UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
  if (LoopingSoundComponent) LoopingSoundComponent->Stop();
  bHit = true;
}

void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
                                      AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp,
                                      int32 OtherBodyIndex, bool bFromSweep,
                                      const FHitResult& SweepResult) {
  AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();
  if (SourceAvatarActor == OtherActor) return; // Dont hit yourself.
  if (!UAuraAbilitySystemLibrary::IsNotFriend(SourceAvatarActor, OtherActor)) return;
  if (!bHit) { OnHit(); }
  if (HasAuthority()) {
    // Finish Damage Effect Params, we now know who the target is (our overlapactor) so now we set it.
    if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor)) {
      const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
      DamageEffectParams.DeathImpulse = DeathImpulse;
      DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
      UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
    }
    Destroy();
  } else { bHit = true; }
}
