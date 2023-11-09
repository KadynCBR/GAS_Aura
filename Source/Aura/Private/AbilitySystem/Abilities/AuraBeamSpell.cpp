// Copyright Cherry Tea Games


#include "AbilitySystem/Abilities/AuraBeamSpell.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"

void UAuraBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult) {
  if (HitResult.bBlockingHit) {
    MouseHitLocation = HitResult.ImpactPoint;
    MouseHitActor = HitResult.GetActor();
  } else {
    CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
  }
}

void UAuraBeamSpell::StoreOwnerVariables() {
  if (CurrentActorInfo) {
    OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
    OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
  }
}

void UAuraBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation) {
  check(OwnerCharacter);
  if (OwnerCharacter->Implements<UCombatInterface>()) {
    if (USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter)) {
      TArray<AActor*> ActorsToIgnore;
      ActorsToIgnore.Add(OwnerCharacter);
      FHitResult HitResult;
      const FVector SocketLocation = Weapon->GetSocketLocation(FName("TipSocket")); // Parameter or variable. Change to make more robust.
      UKismetSystemLibrary::SphereTraceSingle(
        OwnerCharacter, 
        SocketLocation, 
        BeamTargetLocation, 
        10.f, 
        TraceTypeQuery1, 
        false, 
        ActorsToIgnore, 
        EDrawDebugTrace::None, 
        HitResult, 
        true);
      if (HitResult.bBlockingHit) {
        MouseHitLocation = HitResult.ImpactPoint;
        MouseHitActor = HitResult.GetActor();
      }
    }
  }
}

void UAuraBeamSpell::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets) {
  TArray<AActor*> ActorsToIgnore;
  ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
  ActorsToIgnore.Add(MouseHitActor);
  TArray<AActor*> OverlappingActors;
  UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(
    GetAvatarActorFromActorInfo(), 
    OverlappingActors, 
    ActorsToIgnore,
    850.f,
    MouseHitActor->GetActorLocation());
  //int32 NumAdditionalTargets = FMath::Min(GetAbilityLevel() - 1, MaxNumShockTargets);
  int32 NumAdditionalTargets = 5;
  UAuraAbilitySystemLibrary::GetClosestTargets(NumAdditionalTargets, OverlappingActors, OutAdditionalTargets, MouseHitActor->GetActorLocation());

}