#include "vex.h"

void driveDistanceIn(double distanceIn, double headingDeg)
{
  // PID、IMU 保角、settle 與 timeout 全部使用原生 JAR Drive 實作。
  chassis.drive_distance(distanceIn, headingDeg);
}

void turnToHeadingDeg(double targetHeadingDeg)
{
  // 使用原生 JAR Drive 的 turn PID 與最短角度誤差處理。
  chassis.turn_to_angle(targetHeadingDeg);
}
