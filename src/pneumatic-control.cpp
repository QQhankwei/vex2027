#include "vex.h"

namespace
{
bool ringRejectActive = false;
bool intakeClampActive = false;
bool scoringPistonActive = false;
bool alignmentPistonActive = false;
}

void setRingReject(bool active)
{
  ringRejectActive = active;
  ringRejectPiston.set(active);
}

void setIntakeClamp(bool active)
{
  intakeClampActive = active;
  intakeClampPiston.set(active);
}

void setScoringPiston(bool active)
{
  scoringPistonActive = active;
  scoringPiston.set(active);
}

void setAlignmentPiston(bool active)
{
  alignmentPistonActive = active;
  alignmentPiston.set(active);
}

void toggleRingReject() { setRingReject(!ringRejectActive); }
void toggleIntakeClamp() { setIntakeClamp(!intakeClampActive); }
void toggleScoringPiston() { setScoringPiston(!scoringPistonActive); }
void toggleAlignmentPiston() { setAlignmentPiston(!alignmentPistonActive); }

void updateScoringPistonHold(bool pressed)
{
  // Hold 只暫時覆蓋實際輸出，不改變 scoringPistonActive 保存的 Toggle 狀態。
  // 因此放開 Up 後，可以正確恢復 Y 鍵先前選擇的 true / false。
  if (pressed)
  {
    scoringPiston.set(true);
  }
  else
  {
    scoringPiston.set(scoringPistonActive);
  }
}

void resetPneumatics()
{
  setRingReject(false);
  setIntakeClamp(false);
  setScoringPiston(false);
  setAlignmentPiston(false);
}
