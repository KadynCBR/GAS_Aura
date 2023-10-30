// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystemComponent.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AuraAbilityTypes.h"
#include "GameplayEffectTypes.h"
#include "Game/AuraGameModeBase.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject) {
  if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0)) { // get local player controller
    if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD())) {
      AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
      UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
      UAttributeSet* AS = PS->GetAttributeSet();
      const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
      return AuraHUD->GetOverlayWidgetController(WidgetControllerParams);
    }
  }
  return nullptr;
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject) {
  if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0)) { // get local player controller
    if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD())) {
      AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
      UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
      UAttributeSet* AS = PS->GetAttributeSet();
      const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
      return AuraHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
    }
  }
  return nullptr;
}

void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC) {

  AActor* AvatarActor = ASC->GetAvatarActor();

  UCharacterClassInfo* ClassInfo = GetCharacterClassInfo(WorldContextObject);
  FCharacterClassDefaultInfo ClassDefaultInfo = ClassInfo->GetClassDefaultInfo(CharacterClass);
  // Primary
  FGameplayEffectContextHandle PrimaryEffectContextHandle = ASC->MakeEffectContext();
  PrimaryEffectContextHandle.AddSourceObject(AvatarActor);
  FGameplayEffectSpecHandle PrimaryEffectSpec = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryEffectContextHandle);
  ASC->ApplyGameplayEffectSpecToSelf(*PrimaryEffectSpec.Data.Get());
  // Secondary
  FGameplayEffectContextHandle SecondaryEffectContextHandle = ASC->MakeEffectContext();
  SecondaryEffectContextHandle.AddSourceObject(AvatarActor);
  FGameplayEffectSpecHandle SecondaryEffectSpec = ASC->MakeOutgoingSpec(ClassInfo->SecondaryAttributes, Level, SecondaryEffectContextHandle);
  ASC->ApplyGameplayEffectSpecToSelf(*SecondaryEffectSpec.Data.Get());
  // Vital
  FGameplayEffectContextHandle VitalEffectContextHandle = ASC->MakeEffectContext();
  VitalEffectContextHandle.AddSourceObject(AvatarActor);
  FGameplayEffectSpecHandle VitalEffectSpec = ASC->MakeOutgoingSpec(ClassInfo->VitalAttributes, Level, VitalEffectContextHandle);
  ASC->ApplyGameplayEffectSpecToSelf(*VitalEffectSpec.Data.Get());
}

// Used to give enemies startup abilities. For player look at UAuraAbilitySystemComponent::AddCharacterAbilities
void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass) {
  UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
  if (CharacterClassInfo == nullptr) return;
  for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities) {
    FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
    ASC->GiveAbility(AbilitySpec);
  }
  for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->GetClassDefaultInfo(CharacterClass).StartupAbilities) {
    if (ASC->GetAvatarActor()->Implements<UCombatInterface>()) {
      FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));
      ASC->GiveAbility(AbilitySpec);
    }
  }
}

UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject) {
  AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
  if (!AuraGameMode) return nullptr;
  return AuraGameMode->CharacterClassInfo;
}

bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle) {
  if (const FAuraGameplayEffectContext* AuraContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    return AuraContext->IsBlockedHit();
  }
  return false;
}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle) {
  if (const FAuraGameplayEffectContext* AuraContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    return AuraContext->IsCriticalHit();
  }
  return false;
}

void UAuraAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit) {
  if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    AuraEffectContext->SetIsBlockedHit(bInIsBlockedHit);
  }
}

void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit) {
  if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    AuraEffectContext->SetIsCriticalHit(bInIsCriticalHit);
  }
}

void UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(
  const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, 
  float Radius, const FVector& SphereOrigin) {
  FCollisionQueryParams SphereParams;
  SphereParams.AddIgnoredActors(ActorsToIgnore);
  TArray<FOverlapResult> Overlaps;
  if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)) {
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
    for (FOverlapResult& Overlap : Overlaps) {
      if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor())) {
        OutOverlappingActors.AddUnique(Overlap.GetActor()); 
      }
    }
  }
}

bool UAuraAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor) {
  const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));
  const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));
  const bool bFriends = bBothArePlayers || bBothAreEnemies;
  return !bFriends;
}

float UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel) {
  AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
  if (!AuraGameMode) return 0.0f;
  UCharacterClassInfo* CharacterclassInfo = AuraGameMode->CharacterClassInfo;
  const FCharacterClassDefaultInfo& ClassDefaultInfo = CharacterclassInfo->GetClassDefaultInfo(CharacterClass);
  return ClassDefaultInfo.XPReward.GetValueAtLevel (CharacterLevel);
}
