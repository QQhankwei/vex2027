#pragma once

/** 場地座標。X 向右、Y 向前，角度 0 度代表機器人朝向 +Y。 */
struct Pose2D
{
  double xIn;
  double yIn;
  double headingDeg;
};

/**
 * Tracking Wheels 座標計算核心。
 *
 * 類別不直接綁定任何 Port。呼叫者只要提供兩個追蹤輪的累積距離（英吋）
 * 與 IMU 的累積角度（度），就能重用同一套座標計算。
 */
class TrackingOdometry
{
public:
  TrackingOdometry(double forwardWheelOffsetIn,
                   double sidewaysWheelOffsetIn);

  void reset(double xIn,
             double yIn,
             double headingDeg,
             double forwardWheelDistanceIn,
             double sidewaysWheelDistanceIn);

  void update(double forwardWheelDistanceIn,
              double sidewaysWheelDistanceIn,
              double headingDeg);

  Pose2D pose() const;

private:
  double forwardWheelOffsetIn_;
  double sidewaysWheelOffsetIn_;
  double previousForwardDistanceIn_;
  double previousSidewaysDistanceIn_;
  Pose2D pose_;
};

/** 將 Rotation Sensor 的角度換算成追蹤輪走過的英吋數。 */
double trackingDegreesToInches(double sensorDegrees,
                               double wheelDiameterIn);
