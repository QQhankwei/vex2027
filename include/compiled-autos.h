#pragma once

#include <cstddef>

// Auto Studio 自動產生：所有已儲存的 Path 與 Auto 都會一起編譯。
namespace compiledAutos
{
enum class DriveType { Tank, Mecanum };
enum class AutoStepType { FollowPath, Wait, NamedCommand };
struct PathPoint { double xIn; double yIn; double headingDeg; double velocityPct; double tangentDeg; double tangentStrengthPct; };
struct PathDefinition { const char *name; DriveType driveType; const PathPoint *points; std::size_t pointCount; };
struct AutoStep { AutoStepType type; const char *target; double value; };
struct AutoDefinition { const char *name; const AutoStep *steps; std::size_t stepCount; };

inline constexpr PathPoint path_skills_path[]{
  PathPoint{133.330, 9.410, 0.00, 60.0, 0.00, 100.0},
  PathPoint{120.100, 76.140, -37.21, 60.0, -37.21, 180.0},
  PathPoint{94.720, 60.250, -68.08, 60.0, -68.08, 100.0},
  PathPoint{85.760, 89.960, -24.54, 60.0, -24.54, 100.0},
  PathPoint{64.790, 125.790, -30.34, 60.0, -30.34, 100.0}
};

inline constexpr PathPoint path_test[]{
  PathPoint{12.530, 16.220, -3.88, 100.0, -3.88, 180.0},
  PathPoint{45.720, 42.950, 25.64, 100.0, 25.64, 100.0},
  PathPoint{45.720, 85.360, 13.88, 100.0, 13.88, 100.0},
  PathPoint{60.190, 101.520, 41.70, 60.0, 41.70, 100.0},
  PathPoint{72.810, 115.760, 107.48, 95.0, 107.48, 100.0},
  PathPoint{95.130, 90.520, 67.38, 100.0, 67.38, 64.7},
  PathPoint{113.580, 67.550, 141.23, 100.0, 141.23, 100.0}
};

inline constexpr AutoStep auto_skills_auto[]{
  AutoStep{AutoStepType::FollowPath, "skills-path", 0.000},
  AutoStep{AutoStepType::NamedCommand, "intake-forward", 0.000},
  AutoStep{AutoStepType::Wait, "", 0.250},
  AutoStep{AutoStepType::NamedCommand, "stop-all", 0.000}
};

inline constexpr AutoStep auto_skitutorial_auto[]{
  AutoStep{AutoStepType::NamedCommand, "intake-forward", 0.000},
  AutoStep{AutoStepType::FollowPath, "skills-path", 0.000},
  AutoStep{AutoStepType::NamedCommand, "intake-stop", 0.000},
  AutoStep{AutoStepType::NamedCommand, "clamp-on", 0.000},
  AutoStep{AutoStepType::Wait, "", 0.250},
  AutoStep{AutoStepType::NamedCommand, "stop-all", 0.000}
};

inline constexpr AutoStep auto_tutorial_auto[]{
  AutoStep{AutoStepType::NamedCommand, "intake-forward", 0.000},
  AutoStep{AutoStepType::FollowPath, "test", 0.000},
  AutoStep{AutoStepType::NamedCommand, "intake-stop", 0.000},
  AutoStep{AutoStepType::NamedCommand, "clamp-on", 0.000},
  AutoStep{AutoStepType::Wait, "", 0.250},
  AutoStep{AutoStepType::NamedCommand, "stop-all", 0.000}
};

inline constexpr PathDefinition paths[]{
  PathDefinition{"skills-path", DriveType::Tank, path_skills_path, sizeof(path_skills_path) / sizeof(path_skills_path[0])},
  PathDefinition{"test", DriveType::Tank, path_test, sizeof(path_test) / sizeof(path_test[0])}
};
inline constexpr AutoDefinition autos[]{
  AutoDefinition{"skills-auto", auto_skills_auto, sizeof(auto_skills_auto) / sizeof(auto_skills_auto[0])},
  AutoDefinition{"skitutorial-auto", auto_skitutorial_auto, sizeof(auto_skitutorial_auto) / sizeof(auto_skitutorial_auto[0])},
  AutoDefinition{"tutorial-auto", auto_tutorial_auto, sizeof(auto_tutorial_auto) / sizeof(auto_tutorial_auto[0])}
};
}
