#include "vex.h"

namespace
{
constexpr double PI = 3.14159265358979323846;

double degreesToRadians(double degrees)
{
  return degrees * PI / 180.0;
}
}

TrackingOdometry::TrackingOdometry(double forwardWheelOffsetIn,
                                   double sidewaysWheelOffsetIn)
  : forwardWheelOffsetIn_(forwardWheelOffsetIn),
    sidewaysWheelOffsetIn_(sidewaysWheelOffsetIn),
    previousForwardDistanceIn_(0.0),
    previousSidewaysDistanceIn_(0.0),
    pose_{0.0, 0.0, 0.0}
{
}

void TrackingOdometry::reset(double xIn,
                             double yIn,
                             double headingDeg,
                             double forwardWheelDistanceIn,
                             double sidewaysWheelDistanceIn)
{
  pose_ = {xIn, yIn, headingDeg};
  previousForwardDistanceIn_ = forwardWheelDistanceIn;
  previousSidewaysDistanceIn_ = sidewaysWheelDistanceIn;
}

void TrackingOdometry::update(double forwardWheelDistanceIn,
                              double sidewaysWheelDistanceIn,
                              double headingDeg)
{
  const double forwardDeltaIn =
    forwardWheelDistanceIn - previousForwardDistanceIn_;
  const double sidewaysDeltaIn =
    sidewaysWheelDistanceIn - previousSidewaysDistanceIn_;
  const double headingDeltaRad =
    degreesToRadians(headingDeg - pose_.headingDeg);

  previousForwardDistanceIn_ = forwardWheelDistanceIn;
  previousSidewaysDistanceIn_ = sidewaysWheelDistanceIn;

  double localXIn = sidewaysDeltaIn;
  double localYIn = forwardDeltaIn;

  // 轉彎時追蹤輪本身也會繞機器人中心畫圓，因此需扣除 offset 造成的位移。
  if (std::fabs(headingDeltaRad) > 0.000001)
  {
    localXIn = 2.0 * std::sin(headingDeltaRad / 2.0) *
      (sidewaysDeltaIn / headingDeltaRad + sidewaysWheelOffsetIn_);
    localYIn = 2.0 * std::sin(headingDeltaRad / 2.0) *
      (forwardDeltaIn / headingDeltaRad + forwardWheelOffsetIn_);
  }

  // 使用這一個更新週期的中間角度，把機器人局部位移旋轉到場地座標。
  const double averageHeadingRad =
    degreesToRadians(pose_.headingDeg) + headingDeltaRad / 2.0;
  pose_.xIn += localXIn * std::cos(averageHeadingRad) +
               localYIn * std::sin(averageHeadingRad);
  pose_.yIn += localYIn * std::cos(averageHeadingRad) -
               localXIn * std::sin(averageHeadingRad);
  pose_.headingDeg = headingDeg;
}

Pose2D TrackingOdometry::pose() const
{
  return pose_;
}

double trackingDegreesToInches(double sensorDegrees,
                               double wheelDiameterIn)
{
  return (sensorDegrees / 360.0) * PI * wheelDiameterIn;
}
