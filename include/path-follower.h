#pragma once

enum class PathFollowResult
{
  Completed,
  InvalidPath,
  UnsupportedDrivetrain,
  ExcessiveDeviation,
  TimedOut
};

/** 依名稱連續追蹤已編譯路徑；起點 Pose 取自第一個 waypoint。 */
PathFollowResult followPath(const char *pathName);
