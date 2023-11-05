// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AuraGameplayTags.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AuraAbilityTypes.h"

struct AuraDamageStatics {
  
  DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
  DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
  DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
  DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
  DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);
  DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);

  DECLARE_ATTRIBUTE_CAPTUREDEF(ResistFire);
  DECLARE_ATTRIBUTE_CAPTUREDEF(ResistLightning);
  DECLARE_ATTRIBUTE_CAPTUREDEF(ResistArcane);
  DECLARE_ATTRIBUTE_CAPTUREDEF(ResistPhysical);


  AuraDamageStatics() {
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);  
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);  
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);  
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);  
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false); 
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false);

    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ResistFire, Target, false); 
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ResistLightning, Target, false); 
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ResistArcane, Target, false); 
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ResistPhysical, Target, false); 
  

  }
};

static const AuraDamageStatics& DamageStatics() {
  static AuraDamageStatics DStatics;
  return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage() {
  RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
  RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
  RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
  RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
  RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
  RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);

  RelevantAttributesToCapture.Add(DamageStatics().ResistFireDef);
  RelevantAttributesToCapture.Add(DamageStatics().ResistLightningDef);
  RelevantAttributesToCapture.Add(DamageStatics().ResistArcaneDef);
  RelevantAttributesToCapture.Add(DamageStatics().ResistPhysicalDef);

}

void UExecCalc_Damage::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const {

  TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;
  const FAuraGameplayTags Tags = FAuraGameplayTags::Get();
  TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, DamageStatics().ArmorDef);
  TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, DamageStatics().ArmorPenetrationDef);
  TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance, DamageStatics().BlockChanceDef);
  TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitChance, DamageStatics().CriticalHitChanceDef);
  TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitResistance, DamageStatics().CriticalHitResistanceDef);
  TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitDamage, DamageStatics().CriticalHitDamageDef);
  TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, DamageStatics().ResistFireDef);
  TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning, DamageStatics().ResistLightningDef);
  TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane, DamageStatics().ResistArcaneDef);
  TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, DamageStatics().ResistPhysicalDef);

  // Can get a lot of stuff
  const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
  const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

  AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
  AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

  // I dont actually use playerlevel here, but incase i do one day.
  int32 SourcePlayerLevel = 1;
  if (SourceAvatar && SourceAvatar->Implements<UCombatInterface>()) {
    SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
  }
  int32 TargetPlayerLevel = 1;
  if (TargetAvatar && TargetAvatar->Implements<UCombatInterface>()) {
    TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
  }

  const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

  FAggregatorEvaluateParameters EvaluationParameters;
  EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
  EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
  const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

  DetermineDebuff(TagsToCaptureDefs, ExecutionParams, Spec, EvaluationParameters);

  FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
  // Get damage set by caller magnitude
  float Damage = 0.f;

  for (auto& Pair : GameplayTags.DamageTypesToResistances) {
    const FGameplayTag DamageType = Pair.Key;
    const FGameplayTag ResistanceTag = Pair.Value;
    checkf(TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagstoCaptureDefs doesn't contain Tag: [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
    const FGameplayEffectAttributeCaptureDefinition CaptureDef = TagsToCaptureDefs[ResistanceTag];
    float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key, false, 0);
    float Resistance = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, Resistance);
    Resistance = FMath::Clamp(Resistance, 0.f, 100.f);
    DamageTypeValue *= (100.f - Resistance) / 100.f;
    Damage += DamageTypeValue;
  }
  float BlockChance = 0.f;
  ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluationParameters, BlockChance);
  BlockChance = FMath::Max<float>(0.f, BlockChance);
  // If Block successful, half damage.
  const bool bBlocked = FMath::RandRange(0.f, 100.f) < BlockChance;
  Damage = bBlocked ? Damage/2.f : Damage;
  UAuraAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);

  float TargetArmor = 0.f;
  ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
  TargetArmor = FMath::Max<float>(0.f, TargetArmor);

  float SourceArmorPenetration = 0.f;
  ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluationParameters, SourceArmorPenetration);
  SourceArmorPenetration = FMath::Max<float>(0.f, SourceArmorPenetration);

  // Ignores a percentage of the Target's Armor
  const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * 0.25f) / 100.f;
  // Every 3 points of Effective Armor reduces damage by 1%
  Damage *= (100 - EffectiveArmor * 0.333f) / 100.f;

  float SourceCriticalChance = 0.f;
  ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluationParameters, SourceCriticalChance);
  SourceCriticalChance = FMath::Max<float>(0.f, SourceCriticalChance);

  float TargetCriticalResist = 0.f;
  ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluationParameters, TargetCriticalResist);
  TargetCriticalResist = FMath::Max<float>(0.f, TargetCriticalResist);

  float SourceCriticalHitDamage = 0.f;
  ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluationParameters, SourceCriticalHitDamage);
  SourceCriticalHitDamage = FMath::Max<float>(0.f, SourceCriticalHitDamage);

  // Every 4 points of critical resist reduces critical chance by 1%
  // If crit, add bonus crit damage
  const float EffectiveCrit = SourceCriticalChance - (TargetCriticalResist * 0.25f);
  const bool bCritical = FMath::RandRange(0.f, 100.f) < EffectiveCrit;
  Damage = bCritical ? (Damage * 2) + SourceCriticalHitDamage : Damage;
  UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCritical);

  FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
  OutExecutionOutput.AddOutputModifier(EvaluatedData);
}

void UExecCalc_Damage::DetermineDebuff(const TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& TagsToCaptureDefs, const FGameplayEffectCustomExecutionParameters & ExecutionParams, const FGameplayEffectSpec & Spec, FAggregatorEvaluateParameters & EvaluationParameters) const
{
  const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
  for (auto& Pair : GameplayTags.DamageTypesToDebuffs) {
    const FGameplayTag& DamageType = Pair.Key;
    const FGameplayTag& DebuffType = Pair.Value;
    const float TypeDamage = Spec.GetSetByCallerMagnitude(DamageType, false, -1.f);

    if (TypeDamage > -0.5f) { // .5 padding for floating point inprecision
      // Determine if there was a successful debuff
      const float SourceDebuffChance = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Chance, false, -1.f);
      const FGameplayTag& ResistanceTag = GameplayTags.DamageTypesToResistances[DamageType];
      float TargetDebuffResistance = 0.f;
      ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(TagsToCaptureDefs[ResistanceTag], EvaluationParameters, TargetDebuffResistance);
      TargetDebuffResistance = FMath::Max<float>(TargetDebuffResistance, 0.f);
      const float EffectiveDebuffChance = SourceDebuffChance * (100 - TargetDebuffResistance) / 100.f; // each point of resistance reduces chance of debuff by 1%
      const bool bDebuff = FMath::RandRange(1, 100) < EffectiveDebuffChance;
      if (bDebuff) {
        // Package all this stuff to unpack later in Aura attribute set.
        FGameplayEffectContextHandle ContextHandle = Spec.GetContext();
        UAuraAbilitySystemLibrary::SetIsSuccessfulDebuff(ContextHandle, bDebuff);
        UAuraAbilitySystemLibrary::SetDamageType(ContextHandle, DamageType);
        const float DebuffDamage = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Damage, false, -1.f);
        const float DebuffDuration = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Duration, false, -1.f);
        const float DebuffFrequency = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Frequency, false, -1.f);

        UAuraAbilitySystemLibrary::SetDebuffDamage(ContextHandle, DebuffDamage);
        UAuraAbilitySystemLibrary::SetDebuffDuration(ContextHandle, DebuffDuration);
        UAuraAbilitySystemLibrary::SetDebuffFrequency(ContextHandle, DebuffFrequency);
      }
    }
  }
}