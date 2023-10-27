

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
  if (!bHit && !HasAuthority()) {
    UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
    if (LoopingSoundComponent) LoopingSoundComponent->Stop();
    bHit = true;
  }
  Super::Destroyed();
}

void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
                                      AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp,
                                      int32 OtherBodyIndex, bool bFromSweep,
                                      const FHitResult& SweepResult) {
  // Check damageeffectspechandle is valid because this is only set on server, and not replicated. so if client is running into this, return as well.
  // if the otheractor is the same as the effectcauser(one who launched projectile) return early.
  if (!DamageEffectSpecHandle.Data.IsValid() || DamageEffectSpecHandle.Data.Get()->GetContext().GetEffectCauser() == OtherActor) return;
  if (!UAuraAbilitySystemLibrary::IsNotFriend(DamageEffectSpecHandle.Data.Get()->GetContext().GetEffectCauser(), OtherActor)) return;
  if (!bHit) {
    UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
    if (LoopingSoundComponent) LoopingSoundComponent->Stop();
    bHit = true;
  }

  if (HasAuthority()) {

    if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor)) {
      TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
    }
    Destroy();
  } else {
    bHit = true;  
  }
}
