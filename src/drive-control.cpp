#include "vex.h"

using namespace vex;

namespace
{
int clampPercent(int inputPercent)
{
  if (inputPercent > 100) return 100;
  if (inputPercent < -100) return -100;
  return inputPercent;
}
}

int applyDeadband(int inputPercent)
{
  if (std::abs(inputPercent) < robotParameters::CONTROLLER_DEADBAND_PERCENT)
  {
    return 0;
  }
  return clampPercent(inputPercent);
}

void setDrivePercent(int leftPercent, int rightPercent)
{
  leftDrive.spin(forward, clampPercent(leftPercent), percent);
  rightDrive.spin(forward, clampPercent(rightPercent), percent);
}

void stopDrive()
{
  leftDrive.stop();
  rightDrive.stop();
}
