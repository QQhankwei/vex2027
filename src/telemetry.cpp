#include "vex.h"

#include <cstdio>

using namespace vex;

extern competition Competition;

namespace
{
constexpr int TELEMETRY_PERIOD_MS = 100;

const char *competitionMode()
{
  if (!Competition.isEnabled()) return "disabled";
  if (Competition.isAutonomous()) return "autonomous";
  if (Competition.isDriverControl()) return "driver";
  return "enabled";
}

double averageDrivePosition(motor &front, motor &middle, motor &rear)
{
  return (front.position(degrees) + middle.position(degrees) +
          rear.position(degrees)) / 3.0;
}

double averageDriveVelocity(motor &front, motor &middle, motor &rear)
{
  return (front.velocity(rpm) + middle.velocity(rpm) +
          rear.velocity(rpm)) / 3.0;
}

double hottestDriveMotor()
{
  double hottest = leftDriveFront.temperature(celsius);
  const double temperatures[] = {
    leftDriveMiddle.temperature(celsius), leftDriveRear.temperature(celsius),
    rightDriveFront.temperature(celsius), rightDriveMiddle.temperature(celsius),
    rightDriveRear.temperature(celsius)};
  for (double temperature : temperatures)
    if (temperature > hottest) hottest = temperature;
  return hottest;
}

int telemetryLoop()
{
  while (true)
  {
    // 單行 JSON 便於 Desktop 程式依換行切包，也方便直接在 Terminal 閱讀。
    std::printf(
      "@VEX_TELEMETRY {\"version\":1,\"source\":\"robot\","
      "\"mode\":\"%s\",\"enabled\":%s,\"batteryPct\":%lu,"
      "\"imuHeadingDeg\":%.2f,\"imuCalibrating\":%s,"
      "\"leftEncoderDeg\":%.2f,\"rightEncoderDeg\":%.2f,"
      "\"leftRpm\":%.2f,\"rightRpm\":%.2f,\"hottestDriveC\":%.1f,"
      "\"upperIntakeRpm\":%.2f,\"lowerIntakeRpm\":%.2f,"
      "\"ringReject\":%s,\"intakeClamp\":%s,"
      "\"scoring\":%s,\"alignment\":%s,\"timestampMs\":%lu}\n",
      competitionMode(), Competition.isEnabled() ? "true" : "false",
      static_cast<unsigned long>(Brain.Battery.capacity()), imuSensor.rotation(degrees),
      imuSensor.isCalibrating() ? "true" : "false",
      averageDrivePosition(leftDriveFront, leftDriveMiddle, leftDriveRear),
      averageDrivePosition(rightDriveFront, rightDriveMiddle, rightDriveRear),
      averageDriveVelocity(leftDriveFront, leftDriveMiddle, leftDriveRear),
      averageDriveVelocity(rightDriveFront, rightDriveMiddle, rightDriveRear),
      hottestDriveMotor(), intakeUpperMotor.velocity(rpm),
      intakeLowerMotor.velocity(rpm), ringRejectPiston.value() ? "true" : "false",
      intakeClampPiston.value() ? "true" : "false",
      scoringPiston.value() ? "true" : "false",
      alignmentPiston.value() ? "true" : "false",
      static_cast<unsigned long>(timer::system()));
    task::sleep(TELEMETRY_PERIOD_MS);
  }
  return 0;
}
}

void startTelemetry()
{
  // static 確保只建立一次；即使教學時重複呼叫也不會產生多條資料流。
  static task telemetryTask(telemetryLoop);
  (void) telemetryTask;
}
