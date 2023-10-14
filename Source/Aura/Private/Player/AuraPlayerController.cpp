

#include "Player/AuraPlayerController.h"
#include "Interaction/EnemyInterface.h"
#include "EnhancedInputSubSystems.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Input/AuraInputComponent.h"

AAuraPlayerController::AAuraPlayerController() {
  bReplicates = true;
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

  AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);

}

void AAuraPlayerController::PlayerTick(float DeltaTime) {
  Super::PlayerTick(DeltaTime);
  CursorTrace();
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
  FHitResult CursorHit;
  GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
  if (!CursorHit.bBlockingHit) return;
  LastActor = ThisActor;
  ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());
  // Below handles cases of highlighting and unhighlighting actors based on scenarios that could arise from above.
  if (!LastActor) {
    if (ThisActor) {
      // Case B
      ThisActor->HighlightActor();
    }
  } else {
    if (!ThisActor) {
      LastActor->UnHighlightActor();
    } else {
      if (LastActor != ThisActor) {
        LastActor->UnHighlightActor();
        ThisActor->HighlightActor();
      }
    }
  }
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag) {
  //GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, *InputTag.ToString());
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag) {
  GEngine->AddOnScreenDebugMessage(2, 3.f, FColor::Blue, *InputTag.ToString());
  if(GetASC() == nullptr) return;
  GetASC()->AbilityInputTagReleased(InputTag);
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag) {
  GEngine->AddOnScreenDebugMessage(3, 3.f, FColor::Green, *InputTag.ToString());
  if(GetASC() == nullptr) return;
  GetASC()->AbilityInputTagHeld(InputTag);
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC() {
  if (AuraAbilitySystemComponent == nullptr) {
    AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
  }
  return AuraAbilitySystemComponent;
}
