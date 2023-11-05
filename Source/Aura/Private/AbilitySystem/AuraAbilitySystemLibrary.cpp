// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "GameplayEffectTypes.h"
#include "Aura/Public/AuraGameplayTags.h"
#include "Game/AuraGameModeBase.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"

bool UAuraAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AAuraHUD*& OutAuraHUD) {
  if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0)) { // get local player controller
    OutAuraHUD = Cast<AAuraHUD>(PC->GetHUD());
    if (OutAuraHUD) {
      AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
      UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
      UAttributeSet* AS = PS->GetAttributeSet();
      OutWCParams.AttributeSet = AS;
      OutWCParams.AbilitySystemComponent = ASC;
      OutWCParams.PlayerController = PC;
      OutWCParams.PlayerState = PS;
      return true;
    }
  }
  return false;
}

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject) {
  FWidgetControllerParams WCParams;
  AAuraHUD* AuraHUD;
  if (MakeWidgetControllerParams(WorldContextObject, WCParams, AuraHUD)) {
    return AuraHUD->GetOverlayWidgetController(WCParams);
  }
  return nullptr;
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject) {
  FWidgetControllerParams WCParams;
  AAuraHUD* AuraHUD;
  if (MakeWidgetControllerParams(WorldContextObject, WCParams, AuraHUD)) {
    return AuraHUD->GetAttributeMenuWidgetController(WCParams);
  }
  return nullptr;
}

USpellMenuWidgetController* UAuraAbilitySystemLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject) {
  FWidgetControllerParams WCParams;
  AAuraHUD* AuraHUD;
  if (MakeWidgetControllerParams(WorldContextObject, WCParams, AuraHUD)) {
    return AuraHUD->GetSpellMenuWidgetController(WCParams);
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

UAbilityInfo* UAuraAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject) {
  AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
  if (!AuraGameMode) return nullptr;
  return AuraGameMode->AbilityInfo;
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

bool UAuraAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle) {
  if (const FAuraGameplayEffectContext* AuraContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    return AuraContext->IsSuccessfulDebuff();
  }
  return false;
}

float UAuraAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle) {
  if (const FAuraGameplayEffectContext* AuraContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    return AuraContext->GetDebuffDamage();
  }
  return 0.0f;
}

float UAuraAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle) {
  if (const FAuraGameplayEffectContext* AuraContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    return AuraContext->GetDebuffDuration();
  }
  return 0.0f;
}

float UAuraAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle) {
  if (const FAuraGameplayEffectContext* AuraContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    return AuraContext->IsSuccessfulDebuff();
  }
  return 0.0f;
}

FGameplayTag UAuraAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle) {
  if (const FAuraGameplayEffectContext* AuraContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    if(AuraContext->GetDamageType().IsValid()) {
      return *AuraContext->GetDamageType();
    }
  }
  return FGameplayTag();
}

FVector UAuraAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle) {
  if (const FAuraGameplayEffectContext* AuraContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    return AuraContext->GetDeathImpulse();
  }
  return FVector::ZeroVector;
}

void UAuraAbilitySystemLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsSucessfulDebuff) {
  if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    AuraEffectContext->SetIsSuccessfulDebuff(bInIsSucessfulDebuff);
  }
}

void UAuraAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDebuffDamage) {
  if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    AuraEffectContext->SetDebuffDamage(InDebuffDamage);;
  }
}

void UAuraAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle, float InDebuffDuration) {
  if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    AuraEffectContext->SetDebuffDuration(InDebuffDuration);
  }
}

void UAuraAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle, float InDebuffFrequency) {
  if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    AuraEffectContext->SetDebuffFrequency(InDebuffFrequency);
  }
}

void UAuraAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& DamageTag) {
  if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(DamageTag);
    AuraEffectContext->SetDamageType(DamageType);
  }
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

void UAuraAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle, const FVector& InDeathImpulse) {
if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
    AuraEffectContext->SetDeathImpulse(InDeathImpulse);
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

FGameplayEffectContextHandle UAuraAbilitySystemLibrary::ApplyDamageEffect(FDamageEffectParams DamageEffectParams) {
  FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
  FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
  EffectContextHandle.AddSourceObject(DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor());
  SetDeathImpulse(EffectContextHandle, DamageEffectParams.DeathImpulse);
  FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(
    DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContextHandle);
  UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);
  UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);
  UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);
  UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);
  UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);
  DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
  return EffectContextHandle;
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
