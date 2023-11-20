// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacter.h"
#include "Player/AuraPlayerState.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerController.h"
#include "AuraGameplayTags.h"
#include "NiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI/HUD/AuraHUD.h"

AAuraCharacter::AAuraCharacter() {
  CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
  CameraBoom->SetupAttachment(GetRootComponent());
  CameraBoom->SetUsingAbsoluteRotation(true);
  CameraBoom->bDoCollisionTest = false;

  TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>("TopDownCameraComponent");
  TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
  TopDownCameraComponent->bUsePawnControlRotation = false;


  LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("LevelUp Component");
  LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());
  LevelUpNiagaraComponent->bAutoActivate = false;

  GetCharacterMovement()->bOrientRotationToMovement = true;
  GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f); // how fast character moves.
  GetCharacterMovement()-> bConstrainToPlane = true;
  GetCharacterMovement()->bSnapToPlaneAtStart = true;

  bUseControllerRotationPitch = false;
  bUseControllerRotationRoll = false;
  bUseControllerRotationYaw = false;

  CharacterClass = ECharacterClass::Elementalist;
}

void AAuraCharacter::PossessedBy(AController* NewController) {
  Super::PossessedBy(NewController);
  // Init ability actor info for the server.
  InitAbilityActorInfo();
  AddCharacterAbilities();
}

void AAuraCharacter::OnRep_PlayerState() {
  Super::OnRep_PlayerState(); 
  // Init ability actor info for the Client.
  InitAbilityActorInfo();
}

void AAuraCharacter::AddToXP_Implementation(int32 InXP) {
  AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
  check(AuraPlayerState);
  AuraPlayerState->AddToXP(InXP);
}

void AAuraCharacter::LevelUp_Implementation() {
  // Since this function is only called on the server, we're going to mutlicast
  // to all clients the spawn effect for the specfic character so all clients can see levelups.
  MulticastLevelUpParticles();
}

void AAuraCharacter::MulticastLevelUpParticles_Implementation() const {
  if (IsValid(LevelUpNiagaraComponent)) {
    const FVector CameraLocation = TopDownCameraComponent->GetComponentLocation();
    const FVector NiagaraSystemLocation = LevelUpNiagaraComponent->GetComponentLocation();
    const FRotator ToCameraRotation = (CameraLocation-NiagaraSystemLocation).Rotation();
    LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
    LevelUpNiagaraComponent->Activate(true);
  }
}

int32 AAuraCharacter::GetPlayerLevel_Implementation() { 
  AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
  check(AuraPlayerState);
  return AuraPlayerState->GetPlayerLevel(); 
}

int32 AAuraCharacter::GetXP_Implementation() const {
  AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
  check(AuraPlayerState);
  return AuraPlayerState->GetPlayerXP(); 
}

int32 AAuraCharacter::FindLevelForXP_Implementation(int32 InXP) const {
  AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
  check(AuraPlayerState);
  return AuraPlayerState->LevelUpInfo->GetLevelFromXP(InXP);
}

int32 AAuraCharacter::GetAttributePointsReward_Implementation(int32 InLevel) {
  AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
  check(AuraPlayerState);
  return AuraPlayerState->LevelUpInfo->LevelUpData[InLevel].AttributePointReward;
}

int32 AAuraCharacter::GetSpellPointsReward_Implementation(int32 InLevel) {
  AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
  check(AuraPlayerState);
  return AuraPlayerState->LevelUpInfo->LevelUpData[InLevel].SpellPointReward;
}

void AAuraCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel) {
  AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
  check(AuraPlayerState);
  AuraPlayerState->AddToLevel(InPlayerLevel);

  if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponent())) {
    AuraASC->UpdateAbilityStatuses(AuraPlayerState->GetPlayerLevel()); // On levelup update ability statuses to new level.
  }
}

void AAuraCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints) {
  AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
  check(AuraPlayerState);
  AuraPlayerState->AddToAttributePoints(InAttributePoints);
}

void AAuraCharacter::AddToSpellPoints_Implementation(int32 InSpellPoints) {
  AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
  check(AuraPlayerState);
  AuraPlayerState->AddToSpellPoints(InSpellPoints);
}

int32 AAuraCharacter::GetAttributePoints_Implementation() const {
    AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
  check(AuraPlayerState);
  return AuraPlayerState->GetAttributePoints();
}

int32 AAuraCharacter::GetSpellPoints_Implementation() const {
  AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
  check(AuraPlayerState);
  return AuraPlayerState->GetSpellPoints();
}

// stun tag isn't replicated by default since its a dynamic tag?
// need to manually set and unset so clients can get particle effects.
void AAuraCharacter::OnRep_Stunned() {
  if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)) {
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    FGameplayTagContainer BlockedTags;
    BlockedTags.AddTag(GameplayTags.Player_Block_CursorTrace);
    BlockedTags.AddTag(GameplayTags.Player_Block_InputHeld);
    BlockedTags.AddTag(GameplayTags.Player_Block_InputPressed);
    BlockedTags.AddTag(GameplayTags.Player_Block_InputReleased);
    if (bIsStunned) {
      AuraASC->AddLooseGameplayTags(BlockedTags);
      StunDebuffComponent->Activate();
    } else {
      AuraASC->RemoveLooseGameplayTags(BlockedTags);
      StunDebuffComponent->Deactivate();
    }
  }
}

void AAuraCharacter::OnRep_Burned() {
  if (bIsBurned) {
    BurnDebuffComponent->Activate();
  } else {
    BurnDebuffComponent->Deactivate();
  }
}

void AAuraCharacter::InitAbilityActorInfo() {
  AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
  check(AuraPlayerState);
  AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
  Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
  AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
  AttributeSet = AuraPlayerState->GetAttributeSet();
  OnASCRegistered.Broadcast(AbilitySystemComponent);
  AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AAuraCharacter::StunTagChanged);

  if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController())) {
    if (AAuraHUD* AuraHUD =  Cast<AAuraHUD>(AuraPlayerController->GetHUD())) {
      AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
    }
  }
  InitializeDefaultAttributes();
}
