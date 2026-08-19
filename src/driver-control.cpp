#include "vex.h"

using namespace vex;

void configureDriverButtonCallbacks()
{
  // pressed 只在按鍵由「未按」變成「按下」時觸發一次，適合 toggle。
  primaryController.ButtonRight.pressed(toggleIntakeClamp);

  // X 鍵 Motor Toggle 範例：第一次按下持續運轉，第二次按下停止。
  primaryController.ButtonX.pressed(toggleIntakeForward);

  // Y 鍵 Toggle 範例：
  // 第一次按下：scoringPiston = true
  // 第二次按下：scoringPiston = false
  // 之後每按一次就在 true / false 之間切換。
  primaryController.ButtonY.pressed(toggleScoringPiston);
  primaryController.ButtonDown.pressed(toggleAlignmentPiston);
  primaryController.ButtonLeft.pressed(toggleRingReject);
}

void updateDriverControls()
{
  // ---------------- 底盤 ----------------
  // Tank Drive：左右搖桿分別控制左右輪組。
  const int leftPercent = applyDeadband(
    primaryController.Axis3.position(percent));
  const int rightPercent = applyDeadband(
    primaryController.Axis2.position(percent));
  setDrivePercent(leftPercent, rightPercent);

  // ---------------- 氣動 Hold 範例 ----------------
  // Up 按住時 scoringPiston 暫時為 true；放開後恢復 Y 鍵保存的 Toggle 狀態。
  updateScoringPistonHold(primaryController.ButtonUp.pressing());

  // ---------------- Intake ----------------
  // 依優先權判斷，確保同時按多個按鍵時只有一個明確結果。
  // A / B：只控制 Upper Intake，示範上層馬達可獨立正反轉。
  // A 同時也是最簡單的 Hold 範例：按住運轉，放開後由 else 停止。
  // pressing() 會在按住期間持續回傳 true，不會保存切換狀態。
  if (primaryController.ButtonA.pressing())
  {
    setIntakeMode(IntakeMode::UpperForward);
  }
  else if (primaryController.ButtonB.pressing())
  {
    setIntakeMode(IntakeMode::UpperReverse);
  }
  else if (primaryController.ButtonR1.pressing())
  {
    setIntakeMode(IntakeMode::Forward);
  }
  else if (primaryController.ButtonR2.pressing())
  {
    setIntakeMode(IntakeMode::Reverse);
  }
  else if (primaryController.ButtonL1.pressing())
  {
    setIntakeMode(IntakeMode::LowerForward);
  }
  else if (primaryController.ButtonL2.pressing())
  {
    setIntakeMode(IntakeMode::LowerReverse);
  }
  else if (isIntakeToggleRunning())
  {
    // X 鍵開啟 Toggle 後，即使放開 X，Intake 仍會持續正轉。
    setIntakeMode(IntakeMode::Forward);
  }
  else
  {
    stopIntake();
  }
}

void stopAllSubsystems()
{
  stopDrive();
  stopIntake();
  resetIntakeToggle();
  resetPneumatics();
}
