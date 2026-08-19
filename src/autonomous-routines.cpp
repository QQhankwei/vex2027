#include "vex.h"

using namespace vex;

void runDoNothingAutonomous()
{
  stopAllSubsystems();
}

void runTimedAutonomousExample()
{
  // 開迴路範例：只依時間移動，不會確認實際距離或方向。
  setDrivePercent(35, 35);
  wait(500, msec);
  stopDrive();

  // 自動與手動共用 IntakeMode，不需直接接觸馬達物件。
  setIntakeMode(IntakeMode::Forward);
  wait(500, msec);
  stopIntake();
}

void runClosedLoopAutonomousExample()
{
  // ==================== 原生 JAR-Template PID 設定 ====================
  // 參數順序：max_voltage, kp, ki, kd, starti。
  // drive PID 控制距離；heading PID 在直行時修正左右差速、保持指定角度。
  chassis.set_drive_constants(12, 0.5, 0.004, 5, 20);
  chassis.set_heading_constants(12, 0.25, 0.007, 5, 40);

  // turn PID 專門負責原地轉向；swing PID 供單側固定的 swing turn 使用。
  chassis.set_turn_constants(12, 0.35, 0.001, 3, 90);
  chassis.set_swing_constants(12, 0.5, 0.001, 2, 15);

  // 參數順序：允許誤差、需要穩定的時間、最長執行時間，時間單位為 ms。
  chassis.set_drive_exit_conditions(0.5, 0, 2000);
  chassis.set_turn_exit_conditions(4, 0, 3000);
  chassis.set_swing_exit_conditions(10, 0, 3000);

  // Auto 開始時將目前朝向定義為 0 度，後續角度全部使用絕對 heading。
  setRobotPose(0.0, 0.0, 0.0);
  resetPneumatics();

  // Intake 可以在底盤閉迴路執行期間持續工作。
  setIntakeMode(IntakeMode::Forward);
  driveDistanceIn(24.0, 0.0);

  // Auto 氣壓範例：直接 set true / false，不使用遙控器的 Toggle callback。
  setIntakeClamp(true);
  wait(250, msec);

  turnToHeadingDeg(90.0);
  driveDistanceIn(12.0, 90.0);

  stopIntake();

  // 氣缸伸出 300 ms 後收回。
  setAlignmentPiston(true);
  setScoringPiston(true);
  wait(300, msec);
  setScoringPiston(false);
  setAlignmentPiston(false);

  // 保持 90 度倒退，離開得分機構。
  driveDistanceIn(-6.0, 90.0);

  stopAllSubsystems();
}

void runCoordinateAutonomousExample()
{
  // 座標動作底層仍使用相同的 JAR drive / heading / turn PID。
  // 第一次測試先降低最高電壓，方便觀察座標與方向誤差。
  chassis.set_drive_constants(8, 0.5, 0.004, 5, 20);
  chassis.set_heading_constants(8, 0.25, 0.007, 5, 40);
  chassis.set_turn_constants(8, 0.35, 0.001, 3, 90);
  chassis.set_drive_exit_conditions(0.5, 0, 2500);
  chassis.set_turn_exit_conditions(4, 0, 3000);

  // 將 Auto 起點定義成場地座標 (0, 0)，機器人朝向 +Y。
  setRobotPose(0.0, 0.0, 0.0);

  driveToPoint(0.0, 24.0);
  turnToPoint(24.0, 36.0);
  driveToPose(24.0, 36.0, 90.0);

  // 座標動作與一般 JAR 動作可以混用。
  driveDistanceIn(-6.0, 90.0);
  turnToHeadingDeg(0.0);

  stopAllSubsystems();
}
