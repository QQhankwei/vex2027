#include "vex.h"
#include "compiled-autos.h"

#include <cstring>

#include <algorithm>
#include <array>
#include <cmath>

namespace
{
constexpr double PI = 3.14159265358979323846;
constexpr std::size_t MAX_SAMPLES = 768;
struct Sample { double x; double y; double velocity; double distance; };

double clamp(double value, double minimum, double maximum)
{
  return std::max(minimum, std::min(maximum, value));
}

double pointDistance(double x1, double y1, double x2, double y2)
{
  return std::hypot(x2 - x1, y2 - y1);
}

double bezier(double p0, double p1, double p2, double p3, double t)
{
  const double u = 1.0 - t;
  return u*u*u*p0 + 3.0*u*u*t*p1 + 3.0*u*t*t*p2 + t*t*t*p3;
}

double sampleCurvature(const std::array<Sample, MAX_SAMPLES>& samples,
                       std::size_t count,
                       std::size_t center)
{
  if (count < 3) return 0.0;
  const std::size_t before = center > 3 ? center - 3 : 0;
  const std::size_t after = std::min(count - 1, center + 3);
  if (before == center || after == center) return 0.0;

  const double firstHeading = std::atan2(
    samples[center].x - samples[before].x,
    samples[center].y - samples[before].y);
  const double secondHeading = std::atan2(
    samples[after].x - samples[center].x,
    samples[after].y - samples[center].y);
  double headingDelta = secondHeading - firstHeading;
  while (headingDelta > PI) headingDelta -= 2.0 * PI;
  while (headingDelta < -PI) headingDelta += 2.0 * PI;
  const double arcLength = samples[after].distance - samples[before].distance;
  return arcLength > 0.001 ? headingDelta / arcLength : 0.0;
}

const compiledAutos::PathDefinition *findPath(const char *name)
{
  if (name == nullptr) return nullptr;
  for (const auto& path : compiledAutos::paths)
    if (std::strcmp(name, path.name) == 0) return &path;
  return nullptr;
}

std::size_t buildSamples(const compiledAutos::PathDefinition& path,
                         std::array<Sample, MAX_SAMPLES>& samples)
{
  if (path.pointCount < 2) return 0;
  samples[0] = {path.points[0].xIn, path.points[0].yIn,
                path.points[0].velocityPct, 0.0};
  std::size_t count = 1;

  for (std::size_t i = 0;
       i + 1 < path.pointCount && count < MAX_SAMPLES; ++i)
  {
    const auto& a = path.points[i];
    const auto& b = path.points[i + 1];
    const double chord = pointDistance(a.xIn, a.yIn, b.xIn, b.yIn);
    const int divisions = std::max(8, static_cast<int>(std::ceil(
      chord / robotParameters::PATH_SAMPLE_SPACING_IN)));
    const double aAngle = a.tangentDeg * PI / 180.0;
    const double bAngle = b.tangentDeg * PI / 180.0;
    const double aHandle = chord * a.tangentStrengthPct / 300.0;
    const double bHandle = chord * b.tangentStrengthPct / 300.0;
    const double c1x = a.xIn + std::sin(aAngle) * aHandle;
    const double c1y = a.yIn + std::cos(aAngle) * aHandle;
    const double c2x = b.xIn - std::sin(bAngle) * bHandle;
    const double c2y = b.yIn - std::cos(bAngle) * bHandle;

    for (int n = 1; n <= divisions && count < MAX_SAMPLES; ++n)
    {
      const double t = static_cast<double>(n) / divisions;
      Sample sample{};
      sample.x = bezier(a.xIn, c1x, c2x, b.xIn, t);
      sample.y = bezier(a.yIn, c1y, c2y, b.yIn, t);
      sample.velocity = a.velocityPct + (b.velocityPct - a.velocityPct) * t;
      sample.distance = samples[count - 1].distance + pointDistance(
        samples[count - 1].x, samples[count - 1].y, sample.x, sample.y);
      samples[count++] = sample;
    }
  }
  return count;
}
}

PathFollowResult followPath(const char *pathName)
{
  const compiledAutos::PathDefinition *path = findPath(pathName);
  if (path == nullptr) return PathFollowResult::InvalidPath;
  if (path->driveType != compiledAutos::DriveType::Tank)
  {
    stopDrive();
    return PathFollowResult::UnsupportedDrivetrain;
  }

  std::array<Sample, MAX_SAMPLES> samples{};
  const std::size_t count = buildSamples(*path, samples);
  if (count < 2)
  {
    stopDrive();
    return PathFollowResult::InvalidPath;
  }

  const auto& start = path->points[0];
  setRobotPose(start.xIn, start.yIn, start.headingDeg);
  std::size_t nearest = 0;
  PID headingPid(0.0,
                 robotParameters::PATH_HEADING_KP,
                 robotParameters::PATH_HEADING_KI,
                 robotParameters::PATH_HEADING_KD,
                 robotParameters::PATH_HEADING_START_I_DEG);
  double commandedSpeedPercent = 0.0;
  int excessiveDeviationCycles = 0;
  int elapsedMs = 0;
  const int timeoutMs = robotParameters::PATH_TIMEOUT_BASE_MS +
    static_cast<int>(samples[count - 1].distance *
                     robotParameters::PATH_TIMEOUT_PER_INCH_MS);

  while (elapsedMs < timeoutMs)
  {
    const Pose2D pose = getRobotPose();
    const std::size_t searchEnd = std::min(count, nearest + 80);
    double nearestError = pointDistance(
      pose.xIn, pose.yIn, samples[nearest].x, samples[nearest].y);
    for (std::size_t i = nearest + 1; i < searchEnd; ++i)
    {
      const double error = pointDistance(pose.xIn, pose.yIn,
                                         samples[i].x, samples[i].y);
      if (error < nearestError) { nearest = i; nearestError = error; }
    }

    if (nearestError > robotParameters::PATH_MAX_DEVIATION_IN)
      ++excessiveDeviationCycles;
    else
      excessiveDeviationCycles = 0;
    if (excessiveDeviationCycles >= robotParameters::PATH_DEVIATION_STOP_CYCLES)
    {
      stopDrive();
      return PathFollowResult::ExcessiveDeviation;
    }

    const double remaining = samples[count - 1].distance -
      samples[nearest].distance;
    const double finishError = pointDistance(
      pose.xIn, pose.yIn, samples[count - 1].x, samples[count - 1].y);
    if (remaining <= robotParameters::PATH_FINISH_TOLERANCE_IN &&
        finishError <= robotParameters::PATH_FINISH_TOLERANCE_IN)
    {
      stopDrive();
      // Tank 底盤不能獨立旋轉車身方向；抵達位置後再收斂到最後 waypoint 朝向。
      turnToHeadingDeg(path->points[path->pointCount - 1].headingDeg);
      return PathFollowResult::Completed;
    }

    const double wantedDistance = samples[nearest].distance +
      robotParameters::PATH_LOOKAHEAD_IN;
    std::size_t lookahead = nearest;
    while (lookahead + 1 < count && samples[lookahead].distance < wantedDistance)
      ++lookahead;

    const double dx = samples[lookahead].x - pose.xIn;
    const double dy = samples[lookahead].y - pose.yIn;
    double headingError = std::atan2(dx, dy) * 180.0 / PI - pose.headingDeg;
    while (headingError > 180.0) headingError -= 360.0;
    while (headingError < -180.0) headingError += 360.0;

    double speed = clamp(std::abs(samples[lookahead].velocity),
      robotParameters::PATH_MIN_DRIVE_PERCENT,
      robotParameters::PATH_MAX_DRIVE_PERCENT);

    // 以前視點附近曲率限制速度：直線保留原速度，半徑小的急彎提前降速。
    const double curvature = std::abs(sampleCurvature(samples, count, lookahead));
    const double curveSpeedLimit = std::max<double>(
      robotParameters::PATH_CURVE_MIN_PERCENT,
      robotParameters::PATH_MAX_DRIVE_PERCENT /
        (1.0 + robotParameters::PATH_CURVATURE_SLOWDOWN_GAIN * curvature));
    speed = std::min(speed, curveSpeedLimit);
    if (remaining < 16.0)
      speed = std::max<double>(robotParameters::PATH_MIN_DRIVE_PERCENT,
                              speed * clamp(remaining / 16.0, 0.30, 1.0));

    const double speedChange = clamp(
      speed - commandedSpeedPercent,
      -robotParameters::PATH_ACCEL_LIMIT_PERCENT_PER_CYCLE,
      robotParameters::PATH_ACCEL_LIMIT_PERCENT_PER_CYCLE);
    commandedSpeedPercent += speedChange;

    // 與 JAR drive_distance 相同：IMU heading PID 直接修正左右電壓。
    // 這不是單純 Percent 差速；heading 誤差每個週期都會重新閉迴路計算。
    const double driveVolt = commandedSpeedPercent * 0.12;
    const double headingVolt = clamp(
      static_cast<double>(headingPid.compute(static_cast<float>(headingError))),
      -robotParameters::PATH_HEADING_MAX_VOLT,
      robotParameters::PATH_HEADING_MAX_VOLT);
    leftDrive.spin(forward, clamp(driveVolt + headingVolt, -12.0, 12.0), volt);
    rightDrive.spin(forward, clamp(driveVolt - headingVolt, -12.0, 12.0), volt);
    wait(robotParameters::TASK_PERIOD_MS, msec);
    elapsedMs += robotParameters::TASK_PERIOD_MS;
  }

  stopDrive();
  return PathFollowResult::TimedOut;
}
