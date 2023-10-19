

#include "Player/AuraPlayerController.h"
#include "Interaction/EnemyInterface.h"
#include "EnhancedInputSubSystems.h"
#include "AuraGameplayTags.h"
#include "GameFramework/Character.h"
#include "UI/Widget/DamageTextComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Input/AuraInputComponent.h"
#include "Components/SplineComponent.h"

AAuraPlayerController::AAuraPlayerController() {
  bReplicates = true;
  Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::BeginPlay() {
  Super::BeginPlay();
  check(AuraContext); // Assert aura context is set. if not crash.
  UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
  if (Subsystem) {
    Subsystem->AddMappingContext(AuraContext, 0);
  }
  bShowMouseCursor = true;
  DefaultMouseCursor = EMouseCursor::Default;
  FInputModeGameAndUI InputModeData;
  InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
  InputModeData.SetHideCursorDuringCapture(false);
  SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent() {
  Super::SetupInputComponent();
  UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
  AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
  AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
  AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);
  AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void AAuraPlayerController::PlayerTick(float DeltaTime) {
  Super::PlayerTick(DeltaTime);
  CursorTrace();
  AutoRun();
}

void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool IsBlocked, bool IsCritical) {
  if (IsValid(TargetCharacter) && DamageTextComponentClass) {
    UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
    DamageText->RegisterComponent();
    // Attach and detatch so it positions correctly
    DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    DamageText->SetDamageText(DamageAmount, IsBlocked, IsCritical);
  }
}

void AAuraPlayerController::AutoRun() {
  if (!bAutoRunning) return;
  if (APawn* ControlledPawn = GetPawn()) {
    const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
    const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
    ControlledPawn->AddMovementInput(Direction);
    const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
    if (DistanceToDestination <= AutoRunAcceptanceRadius) bAutoRunning = false;
  }
}

// Camera relative movement
void AAuraPlayerController::Move(const FInputActionValue& InputActionValue) {
  const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
  const FRotator Rotation = GetControlRotation();
  const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
  const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
  const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
  // might be called before this is initialized. so we checkin with if instead of check.
  if (APawn* ControlledPawn = GetPawn<APawn>()) {
    ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
    ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
  }
}

void AAuraPlayerController::CursorTrace() {
  GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
  if (!CursorHit.bBlockingHit) return;
  LastActor = ThisActor;
  ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());
  // Below handles cases of highlighting and unhighlighting actors based on scenarios that could arise from above.
  if (LastActor != ThisActor) {
    if (LastActor) LastActor->UnHighlightActor();
    if (ThisActor) ThisActor->HighlightActor();
  }
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag) {
  if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB)) {
    bTargeting = ThisActor ? true : false;
    bAutoRunning = false;
  }
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag) {
  if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB)) {
    if(GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
    return;
  }
  if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
  if (!bTargeting && !bShiftKeyDown) {
    APawn* ControlledPawn = GetPawn();
    if (FollowTime <= ShortPressThreshold && ControlledPawn) {
      // short press, perform autorun.
      if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination)) {
        Spline->ClearSplinePoints();
        for (const FVector& PointLoc : NavPath->PathPoints) {
          Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
        }
        if (NavPath->PathPoints.Num() > 0) {
          CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num()-1];
          bAutoRunning = true;
        }
      }
    }
    FollowTime = 0.f;
    bTargeting = false;
  }
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag) {
  if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB)) {
    if(GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
    return;
  }
  // if we're targeting something activate the ability.
  // If we're targeting, or holding shift key, we want to assume ability activation.
  if (bTargeting || bShiftKeyDown) {
    if(GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
  } else { // Else we are using the mouse click as a click to move and moving towards mouse click.
    FollowTime += GetWorld()->GetDeltaSeconds();
    if (CursorHit.bBlockingHit) {
      CachedDestination = CursorHit.ImpactPoint;
    }
    if (APawn* ControlledPawn = GetPawn()) {
      const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
      ControlledPawn->AddMovementInput(WorldDirection);
    }
  }
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC() {
  if (AuraAbilitySystemComponent == nullptr) {
    AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
  }
  return AuraAbilitySystemComponent;
}

