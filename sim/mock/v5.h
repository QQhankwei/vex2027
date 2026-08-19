#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdarg>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace vex
{
enum directionType { forward = 1, reverse = -1, fwd = forward };
enum percentUnits { percent };
enum rotationUnits { degrees, deg = degrees };
enum voltageUnits { volt };
enum timeUnits { msec };
enum brakeType { coast, brake, hold };
enum gearSetting { ratio6_1, ratio18_1, ratio36_1 };
enum controllerType { primary, partner };

constexpr int PORT1=1,PORT2=2,PORT3=3,PORT4=4,PORT5=5,PORT6=6,PORT7=7,
  PORT8=8,PORT9=9,PORT10=10,PORT11=11,PORT12=12,PORT13=13,PORT14=14,
  PORT15=15,PORT16=16,PORT17=17,PORT18=18,PORT19=19,PORT20=20,PORT21=21,
  PORT22=22;

namespace simulation
{
struct MotorState
{
  double commandVolt=0;
  double actualVolt=0;  // 簡化的一階馬達響應，避免模擬器出現不實際的瞬間反轉。
  double positionDeg=0;
  brakeType stopping=coast;
};
struct RobotState
{
  double xIn=0;
  double yIn=0;
  double headingDeg=0;
  double leftVolt=0;
  double rightVolt=0;
  double upperIntakeVolt=0;
  double lowerIntakeVolt=0;
  bool ringReject=false;
  bool intakeClamp=false;
  bool scoring=false;
  bool alignment=false;
  double elapsedMs=0;
  double motorRpmLimit=600;
};

std::shared_ptr<MotorState> motorState(int port);
void advance(double milliseconds);
void advanceMecanum(double forwardPercent,double strafePercent,double turnPercent,double milliseconds);
void reset();
void setRobotPose(double xIn, double yIn, double headingDeg);
RobotState state();
void setDigital(int channel, bool active);
double inertialRotation(int port);
void setInertialRotation(int port, double degrees);
void setControllerAxis(int axis, int percentValue);
int controllerAxis(int axis);
void setControllerButton(int button, bool pressed);
bool controllerButton(int button);
void setTelemetryCallback(std::function<void(const RobotState&)> callback);
void setMaximumMotorRpm(double rpm);
void setFieldCollisionsEnabled(bool enabled);
}

class motor
{
public:
  motor() : motor(0, ratio18_1, false) {}
  motor(int port, bool reversed=false) : motor(port, ratio18_1, reversed) {}
  motor(int port, gearSetting gear, bool reversed=false)
    : port_(std::abs(port)), reversed_(reversed), gear_(gear), state_(simulation::motorState(std::abs(port))) {}
  void spin(directionType direction, double value, percentUnits)
  { state_->commandVolt=std::clamp(value,-100.0,100.0)*0.12*static_cast<int>(direction); }
  void spin(directionType direction, double value, voltageUnits)
  { state_->commandVolt=std::clamp(value,-12.0,12.0)*static_cast<int>(direction); }
  void stop(brakeType mode=coast) { state_->commandVolt=0; state_->stopping=mode; }
  double position(rotationUnits) const { return state_->positionDeg; }
  void setStopping(brakeType mode) { state_->stopping=mode; }
  int port() const { return port_; }
private:
  int port_=0;
  bool reversed_=false;
  gearSetting gear_=ratio18_1;
  std::shared_ptr<simulation::MotorState> state_;
};

class motor_group
{
public:
  motor_group()=default;
  template<typename... Motors> explicit motor_group(Motors... motors) : motors_{motors...} {}
  void spin(directionType direction,double value,percentUnits units){for(auto& m:motors_)m.spin(direction,value,units);}
  void spin(directionType direction,double value,voltageUnits units){for(auto& m:motors_)m.spin(direction,value,units);}
  void stop(brakeType mode=coast){for(auto& m:motors_)m.stop(mode);}
  void setStopping(brakeType mode){for(auto& m:motors_)m.setStopping(mode);}
  double position(rotationUnits units) const
  { if(motors_.empty())return 0;double sum=0;for(const auto& m:motors_)sum+=m.position(units);return sum/motors_.size(); }
private:
  std::vector<motor> motors_;
};

class inertial
{
public:
  inertial()=default;
  explicit inertial(int port):port_(port){}
  double rotation(rotationUnits=degrees) const{return simulation::inertialRotation(port_);}
  void setRotation(double value,rotationUnits){simulation::setInertialRotation(port_,value);}
  void calibrate(){}
  bool isCalibrating() const{return false;}
private:int port_=0;
};

class rotation
{
public: rotation()=default; explicit rotation(int port):port_(port){}
  double position(rotationUnits) const{return simulation::motorState(port_)->positionDeg;}
private:int port_=0;
};

struct triport_port { int channel=0; };
class encoder
{
public: encoder()=default; explicit encoder(triport_port port):channel_(port.channel){}
  double position(rotationUnits) const{return 0;}
private:int channel_=0;
};
class triport
{
public: explicit triport(int=PORT22){for(int i=0;i<8;i++)Port[i].channel=i;}
  std::array<triport_port,8> Port;
};

class digital_out
{
public: digital_out()=default; explicit digital_out(triport_port port):channel_(port.channel){}
  void set(bool active){active_=active;simulation::setDigital(channel_,active);}
  bool value() const{return active_;}
private:int channel_=0;bool active_=false;
};

class controller
{
public:
  class axis
  {
  public: explicit axis(int id=0):id_(id){} int position(percentUnits)const{return simulation::controllerAxis(id_);} int value()const{return position(percent);}
  private:int id_=0;
  };
  class button
  {
  public: explicit button(int id=0):id_(id){} bool pressing()const{return simulation::controllerButton(id_);} void pressed(void(*callback)()){callback_=callback;}
    void poll(){bool now=pressing();if(now&&!previous_&&callback_)callback_();previous_=now;}
  private:int id_=0;bool previous_=false;void(*callback_)()=nullptr;
  };
  explicit controller(controllerType=primary):Axis1(1),Axis2(2),Axis3(3),Axis4(4),ButtonA(1),ButtonB(2),ButtonX(3),ButtonY(4),ButtonL1(5),ButtonL2(6),ButtonR1(7),ButtonR2(8),ButtonUp(9),ButtonDown(10),ButtonLeft(11),ButtonRight(12){}
  void pollButtons(){ButtonA.poll();ButtonB.poll();ButtonX.poll();ButtonY.poll();ButtonL1.poll();ButtonL2.poll();ButtonR1.poll();ButtonR2.poll();ButtonUp.poll();ButtonDown.poll();ButtonLeft.poll();ButtonRight.poll();}
  axis Axis1,Axis2,Axis3,Axis4;button ButtonA,ButtonB,ButtonX,ButtonY,ButtonL1,ButtonL2,ButtonR1,ButtonR2,ButtonUp,ButtonDown,ButtonLeft,ButtonRight;
};

class brain
{
public:
  struct screen {
    void clearScreen(){} void printAt(int,int,const char*,...){} void drawRectangle(int,int,int,int){}
    void setFillColor(const char*){} void setPenColor(const char*){}
    void pressed(void(*callback)()){pressedCallback_=callback;} int xPosition()const{return x_;} int yPosition()const{return y_;}
  private:int x_=0,y_=0;void(*pressedCallback_)()=nullptr;
  } Screen;
  struct three_wire { triport_port A{0},B{1},C{2},D{3},E{4},F{5},G{6},H{7}; } ThreeWirePort;
};

class task
{
public: task()=default; explicit task(int(*)()){} static void sleep(int milliseconds){simulation::advance(milliseconds);}
};
inline void wait(double amount,timeUnits){simulation::advance(amount);}

class competition
{
public:void autonomous(void(*)()){}void drivercontrol(void(*)()){}
};
}
