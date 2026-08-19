#include "vex.h"

/**
 * JAR-Template 底盤設定。
 * ZERO_TRACKER_ODOM 會使用底盤 Encoder 計算前向距離；實體 Tracking Wheels
 * 確認後，可將 setup 與 tracker 參數改成對應模式。
 */
Drive chassis(
  ZERO_TRACKER_ODOM,
  leftDrive,
  rightDrive,
  PORT12,
  robotParameters::DRIVE_WHEEL_DIAMETER_IN,
  robotParameters::DRIVE_EXTERNAL_GEAR_RATIO,
  360.0,
  // JAR 以負 Port 表示反向；必須與 robot-config.cpp 的 reversed 設定一致。
  -PORT1, PORT7, PORT2, -PORT8,
  5,
  robotParameters::FORWARD_TRACKING_WHEEL_DIAMETER_IN,
  robotParameters::FORWARD_TRACKING_WHEEL_OFFSET_IN,
  1,
  robotParameters::SIDEWAYS_TRACKING_WHEEL_DIAMETER_IN,
  robotParameters::SIDEWAYS_TRACKING_WHEEL_OFFSET_IN);

void configureChassisConstants()
{
  // 以下沿用原競賽程式 default_constants() 的調參方式與意義。
  chassis.set_drive_constants(12, 0.5, 0.004, 5, 20);
  chassis.set_heading_constants(12, 0.25, 0.007, 5, 40);
  chassis.set_turn_constants(12, 0.35, 0.001, 3, 90);
  chassis.set_swing_constants(12, 0.5, 0.001, 2, 15);

  chassis.set_drive_exit_conditions(0.5, 0, 2000);
  chassis.set_turn_exit_conditions(4, 0, 3000);
  chassis.set_swing_exit_conditions(10, 0, 3000);
}
