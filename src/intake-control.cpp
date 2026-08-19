#include "vex.h"

using namespace vex;

namespace
{
// 這個變數只代表 Toggle 模式的要求，不直接代表馬達此刻是否真的在轉。
// Hold 按鍵仍可在 driver-control.cpp 中取得較高優先權。
bool intakeToggleRunning = false;
}

void stopIntake()
{
  intakeUpperMotor.stop(coast);
  intakeLowerMotor.stop(coast);
}

void setIntakeMode(IntakeMode mode)
{
  // 使用明確狀態而不是散落的 Button 判斷，讓自動與手動程式不會各寫一套。
  switch (mode)
  {
    case IntakeMode::Forward:
      intakeUpperMotor.spin(forward,
        robotParameters::INTAKE_FORWARD_PERCENT, percent);
      intakeLowerMotor.spin(forward,
        robotParameters::INTAKE_FORWARD_PERCENT, percent);
      break;

    case IntakeMode::Reverse:
      intakeUpperMotor.spin(forward,
        robotParameters::INTAKE_REVERSE_PERCENT, percent);
      intakeLowerMotor.spin(forward,
        robotParameters::INTAKE_REVERSE_PERCENT, percent);
      break;

    case IntakeMode::UpperForward:
      intakeUpperMotor.spin(forward,
        robotParameters::INTAKE_FORWARD_PERCENT, percent);
      intakeLowerMotor.stop(coast);
      break;

    case IntakeMode::UpperReverse:
      intakeUpperMotor.spin(forward,
        robotParameters::INTAKE_REVERSE_PERCENT, percent);
      intakeLowerMotor.stop(coast);
      break;

    case IntakeMode::LowerForward:
      intakeUpperMotor.stop(coast);
      intakeLowerMotor.spin(forward,
        robotParameters::INTAKE_LOWER_ONLY_PERCENT, percent);
      break;

    case IntakeMode::LowerReverse:
      intakeUpperMotor.stop(coast);
      intakeLowerMotor.spin(forward,
        -robotParameters::INTAKE_LOWER_ONLY_PERCENT, percent);
      break;

    case IntakeMode::Stopped:
    default:
      stopIntake();
      break;
  }
}

void toggleIntakeForward()
{
  intakeToggleRunning = !intakeToggleRunning;
}

bool isIntakeToggleRunning()
{
  return intakeToggleRunning;
}

void resetIntakeToggle()
{
  intakeToggleRunning = false;
}
