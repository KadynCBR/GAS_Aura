// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraEnemy.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AuraGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AI/AuraAIController.h"
#include "UI/Widget/AuraUserWidget.h"
#include "Components/WidgetComponent.h"
#include "Aura/Aura.h"

AAuraEnemy::AAuraEnemy() {
  GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
  AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
  AbilitySystemComponent->SetIsReplicated(true);
  AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
  AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
  HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
  HealthBar->SetupAttachment(GetRootComponent());

  bUseControllerRotationPitch = false;
  bUseControllerRotationRoll = false;
  bUseControllerRotationYaw = false;
  GetCharacterMovement()->bUseControllerDesiredRotation = true;
}

void AAuraEnemy::HighlightActor() { 
  GetMesh()->SetRenderCustomDepth(true);
  GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
  Weapon->SetRenderCustomDepth(true);
  Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void AAuraEnemy::UnHighlightActor() {
  GetMesh()->SetRenderCustomDepth(false);
  Weapon->SetRenderCustomDepth(false);
}

void AAuraEnemy::PossessedBy(AController* NewController) {
  Super::PossessedBy(NewController);
  // AI is only done on the server, anything clients see is a result of replication.
  if (!HasAuthority()) return;
  AuraAIController = Cast<AAuraAIController>(NewController);
  AuraAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
  AuraAIController->RunBehaviorTree(BehaviorTree);
  AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
  AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
  // Consider changing this to accomodate more enemies or more ai stuff.
  AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);
}

int32 AAuraEnemy::GetPlayerLevel_Implementation() { return Level; }

void AAuraEnemy::Die(const FVector& DeathImpulse) {
  SetLifeSpan(LifeSpan);
  if (AuraAIController) AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
  Super::Die(DeathImpulse);
}

void AAuraEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget) { CombatTarget = InCombatTarget; }

AActor* AAuraEnemy::GetCombatTarget_Implementation() const { return CombatTarget; }

void AAuraEnemy::BeginPlay() {
  Super::BeginPlay();
  GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
  InitAbilityActorInfo();
  if (HasAuthority()) {
    UAuraAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
  }

  if (UAuraUserWidget* AuraUserWidget = Cast<UAuraUserWidget>(HealthBar->GetUserWidgetObject())) {
    AuraUserWidget->SetWidgetController(this);
  }

  UAuraAttributeSet* AuraAS = CastChecked<UAuraAttributeSet>(AttributeSet);
  if (AuraAS) {
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute()).AddLambda(
    [this](const FOnAttributeChangeData& Data){
      OnHealthChanged.Broadcast(Data.NewValue);  
    });
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute()).AddLambda(
    [this](const FOnAttributeChangeData& Data){
      OnMaxHealthChanged.Broadcast(Data.NewValue);  
    });

    // Bind to new tag events.. 
    // Consider moving this to AuraBaseCharacter to accomodate playercharacters also affected by this.
    AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(
      this,
      &AAuraEnemy::HitReactTagChanged
    );

    // Initial values
    OnHealthChanged.Broadcast(AuraAS->GetHealth());
    OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
  }
}

void AAuraEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount) {
  bHitReacting = NewCount > 0;
  GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
  // Being called on server and client, but only server would have aicontroller and blackboard component.
  if (AuraAIController && AuraAIController->GetBlackboardComponent()) {
    AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
  }
}

void AAuraEnemy::InitAbilityActorInfo() {
  AbilitySystemComponent->InitAbilityActorInfo(this, this);
  Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
  if (HasAuthority()) {
    InitializeDefaultAttributes();
  }
  OnASCRegistered.Broadcast(AbilitySystemComponent);
}

void AAuraEnemy::InitializeDefaultAttributes() {
  UAuraAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}


