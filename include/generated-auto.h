#pragma once

#include <array>

// 由 VEX Desktop Simulator Auto Builder 自動產生。
namespace generatedAuto
{
enum class AutoStepType
{
  FollowPath,
  Wait,
  NamedCommand
};

struct AutoStep
{
  AutoStepType type;
  const char *target;
  double value;
};

inline constexpr char name[] = "tutorial-auto";
inline constexpr std::array<AutoStep, 6> steps{{
  AutoStep{AutoStepType::NamedCommand, "intake-forward", 0.000},
  AutoStep{AutoStepType::FollowPath, "test", 0.000},
  AutoStep{AutoStepType::NamedCommand, "intake-stop", 0.000},
  AutoStep{AutoStepType::NamedCommand, "clamp-on", 0.000},
  AutoStep{AutoStepType::Wait, "", 0.250},
  AutoStep{AutoStepType::NamedCommand, "stop-all", 0.000}
}};
}
