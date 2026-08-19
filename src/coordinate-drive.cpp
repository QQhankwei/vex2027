#include "vex.h"

using namespace vex;

namespace
{
constexpr double PI = 3.14159265358979323846;

// 現階段以左右底盤 Encoder 當作前向追蹤來源，側向距離固定為 0。
// 未來接上實體 Tracking Wheels 時，只需替換 readForward / readSideways，
// turnToPoint()、driveToPoint() 與 driveToPose() 的 API 不需要改。
TrackingOdometry robotOdometry(0.0, 0.0);
bool odometryInitialized = false;

double readForwardDistanceIn()
{
  const double averageMotorDegrees =
    (leftDrive.position(degrees) + rightDrive.position(degrees)) / 2.0;
  const double wheelRotations = (averageMotorDegrees / 360.0) *
    robotParameters::DRIVE_EXTERNAL_GEAR_RATIO;
  return wheelRotations * PI * robotParameters::DRIVE_WHEEL_DIAMETER_IN;
}

double readSidewaysDistanceIn()
{
  return 0.0;
}

void updateOdometry()
{
  if (!odometryInitialized)
  {
    setRobotPose(0.0, 0.0, imuSensor.rotation(degrees));
  }

  robotOdometry.update(
    readForwardDistanceIn(),
    readSidewaysDistanceIn(),
    imuSensor.rotation(degrees));
}

double angleToPointDeg(const Pose2D& current,
                       double targetXIn,
                       double targetYIn)
{
  // 本專案 0 度朝 +Y，所以使用 atan2(deltaX, deltaY)。
  return std::atan2(targetXIn - current.xIn,
                    targetYIn - current.yIn) * 180.0 / PI;
}
}

void setRobotPose(double xIn, double yIn, double headingDeg)
{
#ifdef VEX_DESKTOP_SIM
  // 桌面物理 Pose 與 Odometry 使用相同起點，避免預覽和 C++ 執行偏移。
  simulation::setRobotPose(xIn, yIn, headingDeg);
#endif
  imuSensor.setRotation(headingDeg, degrees);
  robotOdometry.reset(
    xIn,
    yIn,
    headingDeg,
    readForwardDistanceIn(),
    readSidewaysDistanceIn());
  odometryInitialized = true;
}

Pose2D getRobotPose()
{
  updateOdometry();
  return robotOdometry.pose();
}

void turnToPoint(double targetXIn, double targetYIn)
{
  const Pose2D current = getRobotPose();
  turnToHeadingDeg(angleToPointDeg(current, targetXIn, targetYIn));
  updateOdometry();
}

void driveToPoint(double targetXIn, double targetYIn)
{
  Pose2D current = getRobotPose();
  const double targetHeadingDeg =
    angleToPointDeg(current, targetXIn, targetYIn);

  turnToHeadingDeg(targetHeadingDeg);
  updateOdometry();

  // 轉向後重新讀取 Pose，再計算剩餘直線距離。
  current = robotOdometry.pose();
  const double deltaXIn = targetXIn - current.xIn;
  const double deltaYIn = targetYIn - current.yIn;
  const double distanceIn = std::sqrt(
    deltaXIn * deltaXIn + deltaYIn * deltaYIn);

  driveDistanceIn(distanceIn, targetHeadingDeg);
  updateOdometry();
}

void driveToPose(double targetXIn,
                 double targetYIn,
                 double finalHeadingDeg)
{
  driveToPoint(targetXIn, targetYIn);
  turnToHeadingDeg(finalHeadingDeg);
  updateOdometry();
}
