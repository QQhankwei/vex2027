#pragma once

#include <array>
#include <cstddef>

// 由 VEX Desktop Simulator Path Planner 自動產生，請回到地圖編輯後重新儲存。
namespace generatedPath
{
enum class DriveType
{
  Tank,
  Mecanum
};

struct PathPoint
{
  double xIn;
  double yIn;
  double headingDeg;
  double velocityPct;
  double tangentDeg;
  double tangentStrengthPct;
};

inline constexpr DriveType driveType = DriveType::Tank;
inline constexpr bool holonomic = false;
inline constexpr double robotWidthIn = 13.00;
inline constexpr double robotLengthIn = 10.00;
inline constexpr std::array<PathPoint, 7> points{{
  PathPoint{12.530, 16.220, -3.88, 100.0, -3.88, 180.0},
  PathPoint{45.720, 42.950, 25.64, 100.0, 25.64, 100.0},
  PathPoint{45.720, 85.360, 13.88, 100.0, 13.88, 100.0},
  PathPoint{60.190, 101.520, 41.70, 60.0, 41.70, 100.0},
  PathPoint{72.810, 115.760, 107.48, 95.0, 107.48, 100.0},
  PathPoint{95.130, 90.520, 67.38, 100.0, 67.38, 64.7},
  PathPoint{113.580, 67.550, 141.23, 100.0, 141.23, 100.0}
}};
}
