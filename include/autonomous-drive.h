#pragma once

/**
 * 新版教學名稱到原生 JAR-Template Drive 的薄包裝。
 * 本檔不實作第二套 PID；所有閉迴路計算都由 chassis 完成。
 */

/** 呼叫 JAR chassis.drive_distance(distance, heading)。 */
void driveDistanceIn(double distanceIn, double headingDeg);

/** 呼叫 JAR chassis.turn_to_angle(angle)。 */
void turnToHeadingDeg(double targetHeadingDeg);
