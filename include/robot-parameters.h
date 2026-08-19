#pragma once

/**
 * 所有可調參數集中於此。
 *
 * 單位命名規則：
 * - 距離：英吋，名稱以 _IN 結尾。
 * - 角度：度，名稱以 _DEG 結尾。
 * - 時間：毫秒，名稱以 _MS 結尾。
 * - 百分比：-100 至 100，名稱以 _PERCENT 結尾。
 * - 電壓：V，名稱以 _VOLT 結尾。
 */
namespace robotParameters
{
  constexpr int CONTROLLER_DEADBAND_PERCENT = 5;
  constexpr int TASK_PERIOD_MS = 20;

  // Intake 的一般操作速度。若機構卡住，應先排除機械問題，不要直接加速。
  constexpr int INTAKE_FORWARD_PERCENT = 100;
  constexpr int INTAKE_REVERSE_PERCENT = -100;
  constexpr int INTAKE_LOWER_ONLY_PERCENT = 100;

  // 暫時沿用舊值，待實機量測後確認。
  constexpr double DRIVE_WHEEL_DIAMETER_IN = 3.75;
  constexpr double DRIVE_EXTERNAL_GEAR_RATIO = 0.66666;

  // Tank Path Follower 參數。先以低風險教學值上機，再依實際底盤量測微調。
  constexpr double DRIVE_TRACK_WIDTH_IN = 12.0;
  constexpr double PATH_LOOKAHEAD_IN = 5.0;
  constexpr double PATH_SAMPLE_SPACING_IN = 1.0;
  constexpr double PATH_FINISH_TOLERANCE_IN = 1.5;
  constexpr int PATH_MIN_DRIVE_PERCENT = 16;
  constexpr int PATH_MAX_DRIVE_PERCENT = 50;
  constexpr int PATH_TIMEOUT_BASE_MS = 3000;
  constexpr int PATH_TIMEOUT_PER_INCH_MS = 140;

  // 路徑行駛時沿用 JAR heading PID 的控制形式：drive ± heading correction。
  constexpr double PATH_HEADING_KP = 0.25;
  constexpr double PATH_HEADING_KI = 0.007;
  constexpr double PATH_HEADING_KD = 5.0;
  constexpr double PATH_HEADING_START_I_DEG = 40.0;
  constexpr double PATH_HEADING_MAX_VOLT = 6.0;

  // 曲率單位為 1/in；半徑越小，速度上限越低。
  constexpr double PATH_CURVATURE_SLOWDOWN_GAIN = 28.0;
  constexpr int PATH_CURVE_MIN_PERCENT = 20;
  // 每個 20 ms 控制週期最多改變的前進輸出，避免進出彎道時瞬間加減速。
  constexpr double PATH_ACCEL_LIMIT_PERCENT_PER_CYCLE = 2.0;
  constexpr double PATH_MAX_DEVIATION_IN = 6.0;
  constexpr int PATH_DEVIATION_STOP_CYCLES = 10;

  // Tracking Wheel 範例參數。以下數值只是教學預設，不能直接當成實機值。
  constexpr double FORWARD_TRACKING_WHEEL_DIAMETER_IN = 2.75;
  constexpr double SIDEWAYS_TRACKING_WHEEL_DIAMETER_IN = 2.75;
  constexpr double FORWARD_TRACKING_WHEEL_OFFSET_IN = 5.20;
  constexpr double SIDEWAYS_TRACKING_WHEEL_OFFSET_IN = 5.50;
}
