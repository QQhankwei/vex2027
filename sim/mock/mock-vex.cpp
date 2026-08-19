#include "v5.h"

namespace vex::simulation
{
namespace
{
std::unordered_map<int,std::shared_ptr<MotorState>> motors;
std::unordered_map<int,double> inertials;
std::array<int,5> axes{};
std::array<bool,13> buttons{};
RobotState robot;
std::function<void(const RobotState&)> telemetryCallback;
std::mutex stateMutex;
constexpr double wheelDiameterIn=3.75;
constexpr double externalRatio=0.66666;
constexpr double trackWidthIn=12.0;
// 實機原地旋轉會受到輪胎側向摩擦、重量與地墊阻力影響；理想差速公式會過快。
constexpr double drivetrainTurnEfficiency=0.42;
double maximumMotorRpm=600.0;
bool fieldCollisionsEnabled=true;
constexpr double motorResponseSeconds=0.085;
// 畫面上的車體是 9 x 9 SVG units；1 unit = 1.44 in，因此半邊長是 6.48 in。
// 不使用外接圓，否則牆與 Goal 形成的通道會被不合理地縮窄。
constexpr double robotHalfSizeIn=6.48;
constexpr double goalRadiusIn=5.0;
// Override 場地、Path Planner 與前端 SVG 共用 0~144 in 絕對座標。
// 車身尺寸只在 translationIsClear() 扣除一次。
constexpr double fieldMinXIn=0.0;
constexpr double fieldMaxXIn=144.0;
constexpr double fieldMinYIn=0.0;
constexpr double fieldMaxYIn=144.0;
constexpr std::array<std::array<double,2>,9> goalsIn{{
  {{24.05,47.95}},{{24.05,96.05}},{{47.95,24.05}},{{96.05,24.05}},
  {{72.00,72.00}},{{47.95,119.95}},{{96.05,119.95}},{{119.95,47.95}},{{119.95,96.05}}
}};

double averageVoltage(std::initializer_list<int> ports)
{
  double sum=0;for(int port:ports)sum+=motorState(port)->actualVolt;return sum/ports.size();
}

bool translationIsClear(double x,double y,double headingDeg)
{
  const double radians=headingDeg*3.14159265358979323846/180.0;
  const double cosine=std::cos(radians),sine=std::sin(radians);
  const double axisExtent=robotHalfSizeIn*(std::abs(cosine)+std::abs(sine));
  if(x<fieldMinXIn+axisExtent||x>fieldMaxXIn-axisExtent||
     y<fieldMinYIn+axisExtent||y>fieldMaxYIn-axisExtent)return false;

  // 將 Goal 圓心轉進車體座標，計算圓形對旋轉方形（OBB）的實際接觸。
  for(const auto& goal:goalsIn)
  {
    const double dx=goal[0]-x,dy=goal[1]-y;
    const double localX=dx*cosine+dy*sine;
    const double localY=-dx*sine+dy*cosine;
    const double closestX=std::clamp(localX,-robotHalfSizeIn,robotHalfSizeIn);
    const double closestY=std::clamp(localY,-robotHalfSizeIn,robotHalfSizeIn);
    if(std::hypot(localX-closestX,localY-closestY)<goalRadiusIn)return false;
  }
  return true;
}
}

std::shared_ptr<MotorState> motorState(int port)
{
  auto& value=motors[port];if(!value)value=std::make_shared<MotorState>();return value;
}

void advance(double milliseconds)
{
  std::lock_guard lock(stateMutex);
  const double dt=milliseconds/1000.0;
  for(auto& [port,motor]:motors)
  {
    // 實體馬達不會在一個 10 ms PID cycle 內由 +12 V 瞬間變成 -12 V。
    const double alpha=1.0-std::exp(-dt/motorResponseSeconds);
    motor->actualVolt+=(motor->commandVolt-motor->actualVolt)*alpha;
    const double rpm=motor->actualVolt/12.0*maximumMotorRpm;
    motor->positionDeg+=rpm*6.0*dt;
  }
  robot.leftVolt=averageVoltage({1,2,3});
  robot.rightVolt=averageVoltage({7,8,9});
  robot.upperIntakeVolt=motorState(14)->commandVolt;
  robot.lowerIntakeVolt=motorState(11)->commandVolt;
  const double circumference=3.14159265358979323846*wheelDiameterIn;
  const double leftSpeed=robot.leftVolt/12.0*maximumMotorRpm*externalRatio*circumference/60.0;
  const double rightSpeed=robot.rightVolt/12.0*maximumMotorRpm*externalRatio*circumference/60.0;
  const double linear=(leftSpeed+rightSpeed)/2.0;
  const double angularRad=(leftSpeed-rightSpeed)/trackWidthIn*drivetrainTurnEfficiency;
  const double previousHeadingDeg=robot.headingDeg;
  const double nextHeadingDeg=previousHeadingDeg+angularRad*180.0/3.14159265358979323846*dt;
  // 貼牆時旋轉也會改變方形車身的外框。若新角度會穿入牆或 Goal，拒絕該次
  // 旋轉，避免先轉進碰撞體後，後續所有微小平移都無法脫困。
  if(!fieldCollisionsEnabled||translationIsClear(robot.xIn,robot.yIn,nextHeadingDeg))
    robot.headingDeg=nextHeadingDeg;
  const double headingRad=robot.headingDeg*3.14159265358979323846/180.0;
  const double nextX=robot.xIn+std::sin(headingRad)*linear*dt;
  const double nextY=robot.yIn+std::cos(headingRad)*linear*dt;
  // 先嘗試完整位移。若斜向碰撞，分別測試 X / Y，保留沿牆方向的速度，
  // 避免車體只因一小部分碰牆就完全黏住。旋轉能力始終保留。
  if(!fieldCollisionsEnabled||translationIsClear(nextX,nextY,robot.headingDeg))
  {
    robot.xIn=nextX;robot.yIn=nextY;
  }
  else
  {
    if(translationIsClear(nextX,robot.yIn,robot.headingDeg))robot.xIn=nextX;
    if(translationIsClear(robot.xIn,nextY,robot.headingDeg))robot.yIn=nextY;
  }
  robot.elapsedMs+=milliseconds;
  robot.motorRpmLimit=maximumMotorRpm;
  inertials[12]=robot.headingDeg;
  if(telemetryCallback)telemetryCallback(robot);
}

void advanceMecanum(double forwardPercent,double strafePercent,double turnPercent,double milliseconds)
{
  std::lock_guard lock(stateMutex);
  const double dt=milliseconds/1000.0;
  const double alpha=1.0-std::exp(-dt/motorResponseSeconds);
  for(auto& [port,motor]:motors)
  {
    motor->actualVolt+=(motor->commandVolt-motor->actualVolt)*alpha;
    const double rpm=motor->actualVolt/12.0*maximumMotorRpm;
    motor->positionDeg+=rpm*6.0*dt;
  }

  forwardPercent=std::clamp(forwardPercent,-100.0,100.0);
  strafePercent=std::clamp(strafePercent,-100.0,100.0);
  turnPercent=std::clamp(turnPercent,-100.0,100.0);
  robot.upperIntakeVolt=motorState(14)->commandVolt;
  robot.lowerIntakeVolt=motorState(11)->commandVolt;

  // Field-Centric: 先把場地搖桿向量旋轉回車體座標，再套用標準 X-drive 四輪公式。
  const double headingRad=robot.headingDeg*3.14159265358979323846/180.0;
  const double robotForward=forwardPercent*std::cos(headingRad)+strafePercent*std::sin(headingRad);
  const double robotStrafe=strafePercent*std::cos(headingRad)-forwardPercent*std::sin(headingRad);
  double frontLeft=robotForward+robotStrafe+turnPercent;
  double rearLeft=robotForward-robotStrafe+turnPercent;
  double frontRight=robotForward-robotStrafe-turnPercent;
  double rearRight=robotForward+robotStrafe-turnPercent;
  const double largest=std::max({100.0,std::abs(frontLeft),std::abs(rearLeft),std::abs(frontRight),std::abs(rearRight)});
  frontLeft=frontLeft/largest*100.0;rearLeft=rearLeft/largest*100.0;
  frontRight=frontRight/largest*100.0;rearRight=rearRight/largest*100.0;

  // 由正規化後輪速反解實際車體速度；組合操作不會超過馬達上限。
  const double realisedForward=(frontLeft+rearLeft+frontRight+rearRight)/4.0;
  const double realisedStrafe=(frontLeft-rearLeft-frontRight+rearRight)/4.0;
  const double realisedTurn=(frontLeft+rearLeft-frontRight-rearRight)/4.0;
  robot.leftVolt=(frontLeft+rearLeft)/2.0*0.12;
  robot.rightVolt=(frontRight+rearRight)/2.0*0.12;

  const double circumference=3.14159265358979323846*wheelDiameterIn;
  const double maximumSpeed=maximumMotorRpm*externalRatio*circumference/60.0;
  const double forwardSpeed=realisedForward/100.0*maximumSpeed;
  const double strafeSpeed=realisedStrafe/100.0*maximumSpeed;
  const double angularRad=realisedTurn/100.0*(2.0*maximumSpeed/trackWidthIn)*drivetrainTurnEfficiency;
  const double nextHeadingDeg=robot.headingDeg+angularRad*180.0/3.14159265358979323846*dt;
  if(!fieldCollisionsEnabled||translationIsClear(robot.xIn,robot.yIn,nextHeadingDeg))robot.headingDeg=nextHeadingDeg;

  const double updatedHeadingRad=robot.headingDeg*3.14159265358979323846/180.0;
  const double velocityX=std::sin(updatedHeadingRad)*forwardSpeed+std::cos(updatedHeadingRad)*strafeSpeed;
  const double velocityY=std::cos(updatedHeadingRad)*forwardSpeed-std::sin(updatedHeadingRad)*strafeSpeed;
  const double nextX=robot.xIn+velocityX*dt,nextY=robot.yIn+velocityY*dt;
  if(!fieldCollisionsEnabled||translationIsClear(nextX,nextY,robot.headingDeg))
  {
    robot.xIn=nextX;robot.yIn=nextY;
  }
  else
  {
    if(translationIsClear(nextX,robot.yIn,robot.headingDeg))robot.xIn=nextX;
    if(translationIsClear(robot.xIn,nextY,robot.headingDeg))robot.yIn=nextY;
  }
  robot.elapsedMs+=milliseconds;robot.motorRpmLimit=maximumMotorRpm;inertials[12]=robot.headingDeg;
  if(telemetryCallback)telemetryCallback(robot);
}

void reset()
{
  // 馬達物件在 main() 前已建立並持有 shared_ptr；不可清空 map，否則全域馬達
  // 仍會寫入舊狀態，而物理引擎讀到的是新建立且永遠為 0 的狀態。
  std::lock_guard lock(stateMutex);
  for(auto& [port,motor]:motors)
  {
    motor->commandVolt=0;
    motor->actualVolt=0;
    motor->positionDeg=0;
    motor->stopping=coast;
  }
  inertials.clear();axes.fill(0);buttons.fill(false);robot={};
}
void setRobotPose(double xIn,double yIn,double headingDeg)
{
  std::lock_guard lock(stateMutex);
  robot.xIn=xIn;robot.yIn=yIn;robot.headingDeg=headingDeg;
  inertials[12]=headingDeg;
}
RobotState state(){std::lock_guard lock(stateMutex);return robot;}
void setDigital(int channel,bool active)
{
  if(channel==0)robot.ringReject=active;else if(channel==1)robot.intakeClamp=active;else if(channel==6)robot.scoring=active;else if(channel==7)robot.alignment=active;
}
double inertialRotation(int port){return inertials[port];}
void setInertialRotation(int port,double degrees){inertials[port]=degrees;if(port==12)robot.headingDeg=degrees;}
void setControllerAxis(int axis,int value){if(axis>=0&&axis<static_cast<int>(axes.size()))axes[axis]=std::clamp(value,-100,100);}
int controllerAxis(int axis){return axis>=0&&axis<static_cast<int>(axes.size())?axes[axis]:0;}
void setControllerButton(int button,bool pressed){if(button>=0&&button<static_cast<int>(buttons.size()))buttons[button]=pressed;}
bool controllerButton(int button){return button>=0&&button<static_cast<int>(buttons.size())?buttons[button]:false;}
void setTelemetryCallback(std::function<void(const RobotState&)> callback){telemetryCallback=std::move(callback);}
void setMaximumMotorRpm(double rpm){maximumMotorRpm=std::clamp(rpm,100.0,600.0);robot.motorRpmLimit=maximumMotorRpm;}
void setFieldCollisionsEnabled(bool enabled){fieldCollisionsEnabled=enabled;}
}
