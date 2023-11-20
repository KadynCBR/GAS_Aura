// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/PlayerInterface.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AuraAbilityTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Player/AuraPlayerController.h"
#include "GameFramework/Character.h"
#include "AuraGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

UAuraAttributeSet::UAuraAttributeSet() {
  const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

  TagsToAttributes.Add(GameplayTags.Attributes_Primary_Strength, GetStrengthAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Primary_Intelligence, GetIntelligenceAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Primary_Resilience, GetResilienceAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Primary_Vigor, GetVigorAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Secondary_Armor, GetArmorAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ArmorPenetration, GetArmorPenetrationAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Secondary_BlockChance, GetBlockChanceAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitChance, GetCriticalHitChanceAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitDamage, GetCriticalHitDamageAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitResistance, GetCriticalHitResistanceAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Secondary_HealthRegeneration, GetHealthRegenerationAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ManaRegeneration, GetManaRegenerationAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxHealth, GetMaxHealthAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxMana, GetMaxManaAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Fire, GetResistFireAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Lightning, GetResistLightningAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Arcane, GetResistArcaneAttribute);
  TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Physical, GetResistPhysicalAttribute);
}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Armor, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitResistance, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ResistFire, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ResistLightning, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ResistArcane, COND_None, REPNOTIFY_Always);
  DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ResistPhysical, COND_None, REPNOTIFY_Always);
}

// Clamps for calculations / queries but doesn't actually clamp final values. 
// Clamping for actual values should be done in postgameplayeffectExecute with an 
// actual SetATTRIBUTE.
void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) {
  Super::PreAttributeChange(Attribute, NewValue);

  if (Attribute == GetHealthAttribute()) {
    NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
  }  
  if (Attribute == GetManaAttribute()) {
    NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
  }
}

void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) {
  Super::PostGameplayEffectExecute(Data);
  FEffectProperties Props;
  SetEffectProperties(Data, Props);
  if (Props.TargetCharacter->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(Props.TargetCharacter)) {
      return;
  }
  if (Data.EvaluatedData.Attribute == GetHealthAttribute()) {
    SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
  }
  if (Data.EvaluatedData.Attribute == GetManaAttribute()) {
    SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
  }
  if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute()) {
    HandleIncomingDamage(Props);
  }
  if (Data.EvaluatedData.Attribute == GetIncomingXPAttribute()) {
    HandleIncomingXP(Props);
  }
}

void UAuraAttributeSet::HandleIncomingDamage(const FEffectProperties& Props) {
  const float LocalIncomingDamage = GetIncomingDamage();
  SetIncomingDamage(0.f);
  if (LocalIncomingDamage > 0.f) {
    const float NewHealth = GetHealth() - LocalIncomingDamage;
    SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
    // Handle Hit Stun
    const bool bFatal = NewHealth <= 0.f;
    ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor);
    if (bFatal) {
      if (CombatInterface) {
        const FVector DeathImpulse = UAuraAbilitySystemLibrary::GetDeathImpulse(Props.EffectContextHandle);
        CombatInterface->Die(DeathImpulse);
      }
      SendXPEvent(Props);
    } else {
      if (Props.TargetCharacter->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsBeingShocked(Props.TargetCharacter)) {
        // Here we're activating an ability which has any tags in tagcontainer.
        // Meaning on our GA_HitReact we have to give it the effects.hitreact tag
        // in order for the below to work.
        FGameplayTagContainer TagContainer;
        TagContainer.AddTag(FAuraGameplayTags::Get().Effects_HitReact);
        Props.TargetASC->TryActivateAbilitiesByTag(TagContainer);
      }
      if (CombatInterface) {
        const FVector KnockbackVector = UAuraAbilitySystemLibrary::GetKnockbackVector(Props.EffectContextHandle);
        if (!KnockbackVector.IsNearlyZero(1.f))ICombatInterface::Execute_ApplyKnockback(Props.TargetAvatarActor, KnockbackVector);
      }
    }
    const bool isBlockedHit = UAuraAbilitySystemLibrary::IsBlockedHit(Props.EffectContextHandle);
    const bool isCriticalHit = UAuraAbilitySystemLibrary::IsCriticalHit(Props.EffectContextHandle);
    ShowFloatingText(Props, LocalIncomingDamage, isBlockedHit, isCriticalHit);
    if (UAuraAbilitySystemLibrary::IsSuccessfulDebuff(Props.EffectContextHandle)) {
      Debuff(Props);
    }
  }
}

void UAuraAttributeSet::HandleIncomingXP(const FEffectProperties& Props) {
  const float LocalIncomingXP = GetIncomingXP();
  SetIncomingXP(0.f); 
  // Source character is the owner, since GA_ListenForEvents applies GEEventBasedEffect, adding to incomingXP
  if (Props.SourceCharacter->Implements<UPlayerInterface>() && Props.SourceCharacter->Implements<UCombatInterface>()) {
    const int32 CurrentLevel = ICombatInterface::Execute_GetPlayerLevel(Props.SourceCharacter);
    const int32 CurrentXP = IPlayerInterface::Execute_GetXP(Props.SourceCharacter);
    const int32 NewLevel = IPlayerInterface::Execute_FindLevelForXP(Props.SourceCharacter, CurrentXP + LocalIncomingXP);
    const int32 NumLevelUps = NewLevel - CurrentLevel;

    if (NumLevelUps > 0) {
      IPlayerInterface::Execute_AddToPlayerLevel(Props.SourceCharacter, NumLevelUps);
      int32 AttributePointsReward = 0;
      int32 SpellPointsReward = 0;
      for (int32 i = 0; i < NumLevelUps; i++) {
        SpellPointsReward += IPlayerInterface::Execute_GetSpellPointsReward(Props.SourceCharacter, CurrentLevel+i);
        AttributePointsReward += IPlayerInterface::Execute_GetAttributePointsReward(Props.SourceCharacter, CurrentLevel+i);
      }
      IPlayerInterface::Execute_AddToAttributePoints(Props.SourceCharacter, AttributePointsReward);
      IPlayerInterface::Execute_AddToSpellPoints(Props.SourceCharacter, SpellPointsReward);
      // Will reflect post attribute change.
      bTopOffHealth = true;
      bTopOffMana = true;
      IPlayerInterface::Execute_LevelUp(Props.SourceCharacter);
    }
    IPlayerInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXP);
  }
}

void UAuraAttributeSet::Debuff(const FEffectProperties& Props) {
  const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
  FGameplayEffectContextHandle EffectContext = Props.SourceASC->MakeEffectContext();
  EffectContext.AddSourceObject(Props.SourceAvatarActor);

  const FGameplayTag DamageType = UAuraAbilitySystemLibrary::GetDamageType(Props.EffectContextHandle);
  const float DebuffDamage = UAuraAbilitySystemLibrary::GetDebuffDamage(Props.EffectContextHandle);
  const float DebuffDuration = UAuraAbilitySystemLibrary::GetDebuffDuration(Props.EffectContextHandle);
  const float DebuffFrequency = UAuraAbilitySystemLibrary::GetDebuffFrequency(Props.EffectContextHandle);

  FString DebuffName = FString::Printf(TEXT("DynamicDebuff_%s"), *DamageType.ToString());
  UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(DebuffName));
  Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
  Effect->Period = DebuffFrequency;
  Effect->DurationMagnitude = FScalableFloat(DebuffDuration);


  // Set the tags that the Targets will recieve !
  const FGameplayTag DebuffTag = GameplayTags.DamageTypesToDebuffs[DamageType];
  Effect->InheritableOwnedTagsContainer.AddTag(DebuffTag);

    // If we're stunned, add a bunch of block tags. 
    // TODO: Consider making a single tag to encompass this functionality?????
  if (DebuffTag.MatchesTagExact(GameplayTags.Debuff_Stun)) {
    Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputHeld);
    Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_CursorTrace);
    Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputPressed);
    Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputReleased);
  }
  Effect->StackingType = EGameplayEffectStackingType::AggregateBySource;
  Effect->StackLimitCount = 1;
  
  // only for one modifier, but if you wanted more, just add to the index and repeat the below process.
  int32 Index = Effect->Modifiers.Num();
  Effect->Modifiers.Add(FGameplayModifierInfo());
  FGameplayModifierInfo& ModifierInfo = Effect->Modifiers[Index];
  ModifierInfo.ModifierMagnitude = FScalableFloat(DebuffDamage);
  ModifierInfo.ModifierOp = EGameplayModOp::Additive;
  ModifierInfo.Attribute = UAuraAttributeSet::GetIncomingDamageAttribute();

  if (FGameplayEffectSpec* MutableSpec = new FGameplayEffectSpec(Effect, EffectContext, 1.f)) {
    FAuraGameplayEffectContext* AuraContext = static_cast<FAuraGameplayEffectContext*>(MutableSpec->GetContext().Get());
    TSharedPtr<FGameplayTag> DebuffDamageType = MakeShareable(new FGameplayTag(DamageType));
    AuraContext->SetDamageType(DebuffDamageType);
    Props.TargetASC->ApplyGameplayEffectSpecToSelf(*MutableSpec);
  }
}

void UAuraAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) {
  Super::PostAttributeChange(Attribute, OldValue, NewValue);

  if (Attribute == GetMaxHealthAttribute() && bTopOffHealth) {
    SetHealth(GetMaxHealth());
    bTopOffHealth = false;
  }
  if (Attribute == GetMaxManaAttribute() && bTopOffMana) {
    SetMana(GetMaxMana());
    bTopOffMana = false;
  }
}

void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const {
  // Source = causer of the effect, Target = target of the effect (owner of this AS)
  Props.EffectContextHandle = Data.EffectSpec.GetContext();
  Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();
  // This is how one might get the causer's information. Could be used to notify causer that ability did hit, or 
  // to update causer effects etc etc. Lots of power here. (UE5 GAS Course - S6-47)
  if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid()) {
    Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
    Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
    if (Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr) {
      if (APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor)) {
        Props.SourceController = Pawn->GetController();
      }
    }
    if (Props.SourceController) {
      Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
    }
  }
  if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid()) {
    Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
    Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
    Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
    Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
  }
}

void UAuraAttributeSet::ShowFloatingText(const FEffectProperties& Props, float Damage, bool IsBlocked, bool IsCritical) {
  // Floating Damage texts
  if (Props.SourceCharacter != Props.TargetCharacter) {


    // // Show to all Clients 
    TArray<AActor*> PlayerActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), PlayerActors);
    for (AActor* Player_Actor : PlayerActors) {
      AAuraPlayerController* PC = Cast<AAuraPlayerController>(Player_Actor);
      AAuraPlayerController* SPC = Cast<AAuraPlayerController>(Props.SourceCharacter->Controller);
      if (PC && SPC) {
        PC->ShowDamageNumber(Damage, Props.TargetCharacter, IsBlocked, IsCritical, PC == SPC);
      } 
      // Show Enemy -> Player damage as a ghost value
      //AAuraPlayerController* TPC = Cast<AAuraPlayerController>(Props.TargetCharacter->Controller);
      //else if (PC && TPC) {
      //  PC->ShowDamageNumber(Damage, Props.TargetCharacter, IsBlocked, IsCritical, PC == TPC); // <- This might need multiplayer testing.
      //}
    }

    // Show to only Instigator
    /*if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.SourceCharacter->Controller)) {
      PC->ShowDamageNumber(Damage, Props.TargetCharacter, IsBlocked, IsCritical, true);
    }*/

  }
}

void UAuraAttributeSet::SendXPEvent(const FEffectProperties& Props) {
  if (Props.TargetAvatarActor->Implements<UCombatInterface>()) {
    const int32 TargetLevel = ICombatInterface::Execute_GetPlayerLevel(Props.TargetAvatarActor);
    const ECharacterClass TargetClass = ICombatInterface::Execute_GetCharacterClass(Props.TargetCharacter);
    const float XPReward = UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(Props.TargetCharacter, TargetClass, TargetLevel);

    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    FGameplayEventData Payload;
    Payload.EventTag = GameplayTags.Attributes_Meta_IncomingXP;
    Payload.EventMagnitude = XPReward;
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.SourceCharacter, GameplayTags.Attributes_Meta_IncomingXP, Payload);
  }
}

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}

void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}

void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Intelligence, OldIntelligence);
}

void UAuraAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Resilience, OldResilience);
}

void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Vigor, OldVigor);
}

void UAuraAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Armor, OldArmor);
}

void UAuraAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArmorPenetration, OldArmorPenetration);
}

void UAuraAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, BlockChance, OldBlockChance);
}

void UAuraAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void UAuraAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}

void UAuraAttributeSet::OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitResistance, OldCriticalHitResistance);
}

void UAuraAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HealthRegeneration, OldHealthRegeneration);
}

void UAuraAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ManaRegeneration, OldManaRegeneration);
}

void UAuraAttributeSet::OnRep_ResistFire(const FGameplayAttributeData& OldResistFire) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ResistFire, OldResistFire);
}

void UAuraAttributeSet::OnRep_ResistLightning(const FGameplayAttributeData& OldResistLightning) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ResistLightning, OldResistLightning);
}

void UAuraAttributeSet::OnRep_ResistArcane(const FGameplayAttributeData& OldResistArcane) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ResistArcane, OldResistArcane);
}

void UAuraAttributeSet::OnRep_ResistPhysical(const FGameplayAttributeData& OldResistPhysical) const {
  GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ResistPhysical, OldResistPhysical);
}

