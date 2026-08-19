#include "vex.h"

using namespace vex;

competition Competition;

/**
 * 賽前初始化。
 * 此階段只校正 IMU，其他子系統確認硬體後再加入。
 */
void preAutonomous()
{
  vexcodeInit();
  configureChassisConstants();

  Brain.Screen.clearScreen();
  Brain.Screen.printAt(10, 30, "IMU calibrating...");

  imuSensor.calibrate();
  while (imuSensor.isCalibrating())
  {
    wait(robotParameters::TASK_PERIOD_MS, msec);
  }

  initializeAutoSelector();
  startTelemetry();
}

/** 執行 Auto Studio 最後一次儲存並產生到 VS Code 的流程。 */
void autonomous()
{
  if (!isAutonomousEnabled())
  {
    stopAllSubsystems();
    Brain.Screen.clearScreen();
    Brain.Screen.printAt(10, 30, "AUTO DISABLED / SAFE");
    return;
  }

  const AutoRunResult result = runAuto(selectedAutonomousName());
  if (result != AutoRunResult::Completed)
  {
    // 任一步驟、路徑或 Named Command 失敗，都立即停止所有機構。
    stopAllSubsystems();
    Brain.Screen.printAt(10, 60, "AUTO ERROR: %d", static_cast<int>(result));
  }

  // 舊教學 routine 仍保留；測試時只能擇一呼叫，不可與 runAuto() 同時執行：
  // runTimedAutonomousExample();

  // 進階閉迴路範例（Encoder + IMU + PID + Intake + Pneumatic）：
  // runClosedLoopAutonomousExample();

  // 座標範例（目前使用底盤 Encoder + IMU；可換成 Tracking Wheels）：
  // runCoordinateAutonomousExample();
}

/**
 * Tank Drive：左 Axis3 控制左側，右 Axis2 控制右側。
 */
void driverControl()
{
  // Toggle 類按鍵只需註冊一次；不要放進 while 迴圈重複註冊。
  configureDriverButtonCallbacks();

  while (true)
  {
    updateDriverControls();
    wait(robotParameters::TASK_PERIOD_MS, msec);
  }
}

int main()
{
  Competition.autonomous(autonomous);
  Competition.drivercontrol(driverControl);

  preAutonomous();

  while (true)
  {
    // 沒有 Field Control 時，可由 Brain 的 RUN TEST 手動啟動已選 Auto。
    if (takeAutonomousTestRequest()) autonomous();
    wait(100, msec);
  }
}
