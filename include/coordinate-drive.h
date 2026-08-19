#pragma once

/** 將機器人目前位置定義成指定場地座標。 */
void setRobotPose(double xIn, double yIn, double headingDeg);

/** 依 Encoder 與 IMU 更新一次座標，並回傳最新 Pose。 */
Pose2D getRobotPose();

/** 原地轉向，使機器人面向指定座標。 */
void turnToPoint(double targetXIn, double targetYIn);

/** 先面向目標，再以閉迴路定距移動到指定座標。 */
void driveToPoint(double targetXIn, double targetYIn);

/** 移動到指定座標後，再轉到指定最終角度。 */
void driveToPose(double targetXIn,
                 double targetYIn,
                 double finalHeadingDeg);
