#pragma once

/** 預設安全模式：停止全部子系統，不讓尚未驗證的路徑自動執行。 */
void runDoNothingAutonomous();

/**
 * 最小自動程式範例：定時向前、停止、啟動 Intake、停止。
 * 這是開迴路教學範例，不具備距離精度；確認機器周圍安全後才能改為呼叫它。
 */
void runTimedAutonomousExample();

/**
 * 競賽級閉迴路範例：Encoder 定距、IMU 保角、IMU 轉向、Intake 與氣動。
 * 範例先展示原生 JAR constants / exit conditions，再用線性動作清單寫路徑。
 */
void runClosedLoopAutonomousExample();

/** 座標 Auto 範例：設定起點後使用 driveToPoint / driveToPose。 */
void runCoordinateAutonomousExample();
