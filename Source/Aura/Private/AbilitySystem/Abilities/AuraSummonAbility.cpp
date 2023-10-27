// Copyright Cherry Tea Games


#include "AbilitySystem/Abilities/AuraSummonAbility.h"

TArray<FVector> UAuraSummonAbility::GetSpawnLocations() {
  const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
  const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
  const FVector RightOfSpread = Forward.RotateAngleAxis(SpawnSpread/2, FVector::UpVector);
  const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread/2, FVector::UpVector);
  const float DeltaSpread = SpawnSpread / NumMinions;

  TArray<FVector> SpawnLocations;

  for (int32 i = 0;  i < NumMinions; i++) {
    const FVector Direction = LeftOfSpread.RotateAngleAxis((DeltaSpread/2.0)+DeltaSpread * i, FVector::UpVector);
    FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);

    // Linetrace from above to make sure spawning on ground incase of non flat geometry
    FHitResult Hit;
    GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation + FVector(0.f, 0.f, 400.f), ChosenSpawnLocation - FVector(0.f, 0.f, 400.f), ECC_Visibility);

    if (Hit.bBlockingHit) {
      ChosenSpawnLocation = Hit.ImpactPoint;
    }

    SpawnLocations.Add(ChosenSpawnLocation);
  }

  return SpawnLocations;
}

TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass() {
  int32 Selection = FMath::RandRange(0, MinionClasses.Num() - 1);
  return MinionClasses[Selection];
}
