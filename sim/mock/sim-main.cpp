#include "vex.h"
#include "compiled-autos.h"
#include <iostream>
#include <sstream>
#include <string>

namespace
{
std::string activeMode="idle";

void printState(const vex::simulation::RobotState& state,const char* event="state")
{
  std::cout << "{\"event\":\"" << event << "\",\"mode\":\"" << activeMode
    << "\",\"version\":1,\"source\":\"simulator\",\"enabled\":" << (activeMode=="idle"?"false":"true")
    << ",\"batteryPct\":100,\"imuCalibrating\":false"
    << ",\"xIn\":" << state.xIn << ",\"yIn\":" << state.yIn
    << ",\"headingDeg\":" << state.headingDeg << ",\"leftVolt\":" << state.leftVolt
    << ",\"rightVolt\":" << state.rightVolt << ",\"upperIntakeVolt\":" << state.upperIntakeVolt
    << ",\"lowerIntakeVolt\":" << state.lowerIntakeVolt
    << ",\"ringReject\":" << (state.ringReject?"true":"false")
    << ",\"intakeClamp\":" << (state.intakeClamp?"true":"false")
    << ",\"scoring\":" << (state.scoring?"true":"false")
    << ",\"alignment\":" << (state.alignment?"true":"false")
    << ",\"leftEncoderDeg\":" << leftDrive.position(vex::degrees)
    << ",\"rightEncoderDeg\":" << rightDrive.position(vex::degrees)
    << ",\"leftRpm\":" << state.leftVolt/12.0*state.motorRpmLimit
    << ",\"rightRpm\":" << state.rightVolt/12.0*state.motorRpmLimit
    << ",\"hottestDriveC\":" << 25.0+std::max(std::abs(state.leftVolt),std::abs(state.rightVolt))*1.5
    << ",\"upperIntakeRpm\":" << state.upperIntakeVolt/12.0*state.motorRpmLimit
    << ",\"lowerIntakeRpm\":" << state.lowerIntakeVolt/12.0*state.motorRpmLimit
    << ",\"elapsedMs\":" << state.elapsedMs << "}" << std::endl;
}

void initialise()
{
  vex::simulation::reset();
  // 遙控模式必須從完整車身都在牆內的位置開始；(0,0) 是牆角，不是合法車體中心。
  // Generated Auto 之後仍會由路徑第一點呼叫 setRobotPose() 覆蓋此位置。
  vex::simulation::setRobotPose(12.0,12.0,0.0);
  vexcodeInit();configureChassisConstants();
}

void runExampleAuto(const std::string& mode,const std::string& autoName="")
{
  initialise();activeMode=mode;
  // 教學 Auto 範例的 (0,0) 是相對座標，不是 Override 場地上的實際起點；
  // 因此先隔離 PID 測試，不讓示意 Goal 阻擋路徑。遙控模式仍啟用完整碰撞。
  vex::simulation::setFieldCollisionsEnabled(false);
  if(mode=="closed")runClosedLoopAutonomousExample();
  else if(mode=="coordinate")runCoordinateAutonomousExample();
  else if(mode=="timed")runTimedAutonomousExample();
  else if(mode=="generated")
  {
    const char *selectedName=autoName.empty()?compiledAutos::autos[0].name:autoName.c_str();
    const AutoRunResult result=::runAuto(selectedName);
    printState(vex::simulation::state(),
      result==AutoRunResult::Completed?"complete":"auto-error");
    return;
  }
  printState(vex::simulation::state(),"complete");
}
}

int main(int argc,char** argv)
{
  initialise();
  if(argc>1){runExampleAuto(argv[1]);return 0;}
  configureDriverButtonCallbacks();
  vex::simulation::setFieldCollisionsEnabled(true);
  vex::simulation::setTelemetryCallback([](const auto& state){printState(state);});
  printState(vex::simulation::state(),"ready");

  std::string line;
  while(std::getline(std::cin,line))
  {
    std::istringstream input(line);std::string command;input>>command;
    if(command=="drive")
    {
      int left=0,right=0,ms=20;input>>left>>right>>ms;activeMode="driver";
      vex::simulation::setFieldCollisionsEnabled(true);
      vex::simulation::setControllerAxis(3,left);vex::simulation::setControllerAxis(2,right);
      primaryController.pollButtons();updateDriverControls();vex::simulation::advance(ms);
    }
    else if(command=="mecanum")
    {
      int forward=0,strafe=0,turn=0,ms=20;input>>forward>>strafe>>turn>>ms;activeMode="driver";
      vex::simulation::setFieldCollisionsEnabled(true);
      primaryController.pollButtons();updateDriverControls();
      vex::simulation::advanceMecanum(forward,strafe,turn,ms);
    }
    else if(command=="button")
    {
      int id=0,pressed=0;input>>id>>pressed;vex::simulation::setControllerButton(id,pressed!=0);
    }
    else if(command=="tap")
    {
      int id=0;input>>id;
      vex::simulation::setControllerButton(id,true);primaryController.pollButtons();
      vex::simulation::setControllerButton(id,false);primaryController.pollButtons();
      updateDriverControls();vex::simulation::advance(1);
    }
    else if(command=="rpm")
    {
      double rpm=600;input>>rpm;vex::simulation::setMaximumMotorRpm(rpm);
    }
    else if(command=="auto")
    {
      std::string mode,autoName;input>>mode>>autoName;runExampleAuto(mode,autoName);configureDriverButtonCallbacks();
    }
    else if(command=="reset")
    {
      initialise();activeMode="idle";printState(vex::simulation::state(),"ready");
    }
    else if(command=="state")printState(vex::simulation::state());
    else if(command=="quit")break;
  }
}
