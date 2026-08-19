#include "vex.h"

using namespace vex;

brain Brain;
controller primaryController = controller(primary);

// 保留舊專案的 Port、齒輪匣及反轉設定，名稱統一重新定義。
motor leftDriveFront = motor(PORT1, ratio6_1, true);
motor leftDriveMiddle = motor(PORT2, ratio6_1, false);
motor leftDriveRear = motor(PORT3, ratio6_1, true);
motor rightDriveFront = motor(PORT7, ratio6_1, false);
motor rightDriveMiddle = motor(PORT8, ratio6_1, true);
motor rightDriveRear = motor(PORT9, ratio6_1, false);

motor_group leftDrive = motor_group(
  leftDriveFront, leftDriveMiddle, leftDriveRear);
motor_group rightDrive = motor_group(
  rightDriveFront, rightDriveMiddle, rightDriveRear);

inertial imuSensor = inertial(PORT12);

// Intake Port 沿用競賽版本。若實機接線不同，只修改本檔即可。
motor intakeUpperMotor = motor(PORT14, ratio6_1, true);
motor intakeLowerMotor = motor(PORT11, ratio6_1, false);

// Brain 三線接孔的 Digital Out 可接電磁閥。
// true / false 所代表的實際伸出方向，需依氣管接法在實機確認。
digital_out ringRejectPiston = digital_out(Brain.ThreeWirePort.A);
digital_out intakeClampPiston = digital_out(Brain.ThreeWirePort.B);
digital_out scoringPiston = digital_out(Brain.ThreeWirePort.G);
digital_out alignmentPiston = digital_out(Brain.ThreeWirePort.H);

void vexcodeInit()
{
  leftDrive.setStopping(coast);
  rightDrive.setStopping(coast);

  intakeUpperMotor.setStopping(coast);
  intakeLowerMotor.setStopping(coast);

  // 開機時先回到安全狀態，避免程式啟動瞬間誤動作。
  ringRejectPiston.set(false);
  intakeClampPiston.set(false);
  scoringPiston.set(false);
  alignmentPiston.set(false);
}
