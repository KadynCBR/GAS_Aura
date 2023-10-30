// Copyright Cherry Tea Games


#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::GetLevelFromXP(int32 InXP) {
  int32 retlevel = 1;

  for (int i = 1; i < LevelUpData.Num(); i++) {
    if (InXP >= LevelUpData[i].LevelUpRequirement) {
      retlevel = i+1;
    } else {
      break;
    }
  }
  return retlevel;
}

float ULevelUpInfo::GetCurrentXPPercent(int32 InXP) {
  const int32 CurrentLevel = GetLevelFromXP(InXP);
  //const int32 NextLevel = CurrentLevel+1;
  int32 LeveUpXPReq = LevelUpData[CurrentLevel].LevelUpRequirement;
  int32 PreviousLevelXPReq = LevelUpData[CurrentLevel-1].LevelUpRequirement;

  // normalize by previous level's XPrequirement
  int32 DifferenceLevelRequirement = LeveUpXPReq - PreviousLevelXPReq;
  int32 XPForThisLevel = InXP - PreviousLevelXPReq;


  return static_cast<float>(XPForThisLevel) / static_cast<float>(DifferenceLevelRequirement);
}
