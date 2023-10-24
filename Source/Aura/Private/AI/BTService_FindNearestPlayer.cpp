// Copyright Cherry Tea Games


#include "AI/BTService_FindNearestPlayer.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BTFunctionLibrary.h"

void UBTService_FindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
  Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

  APawn* OwningPawn = AIOwner->GetPawn();

  // example code of if the ai is a player, (maybe a pet) then the target will be enemy. otherwise target is players.
  const FName TargetTag = OwningPawn->ActorHasTag(FName("Player")) ? FName("Enemy") : FName("Player");

  TArray<AActor*> ActorsWithTag;
  UGameplayStatics::GetAllActorsWithTag(OwningPawn, TargetTag, ActorsWithTag);

  float distance = TNumericLimits<float>::Max();
  AActor* closestActor = nullptr;
  for (AActor* Actor : ActorsWithTag) {
    if (IsValid(Actor) && IsValid(OwningPawn)) {
      float currentDist = OwningPawn->GetDistanceTo(Actor);
      if (currentDist < distance) {
        closestActor = Actor;
        distance = currentDist;
      }
    }
  }
  UBTFunctionLibrary::SetBlackboardValueAsObject(this, TargetToFollowSelector, closestActor);
  UBTFunctionLibrary::SetBlackboardValueAsFloat(this, DistanceToTargetSelector, distance);
}
