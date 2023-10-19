// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AuraAbilityTypes.h"

struct AuraDamageStatics {
  
  DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
  DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
  DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
  DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
  DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);
  DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);

  AuraDamageStatics() {
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);  
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);  
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);  
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);  
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false); 
    DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false); 
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

}

void UExecCalc_Damage::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const {
  // Can get a lot of stuff
  const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
  const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

  AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
  AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

  const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

  FAggregatorEvaluateParameters EvaluationParameters;
  EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
  EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

  FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
  // Get damage set by caller magnitude
  float Damage = Spec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Damage);
  
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
