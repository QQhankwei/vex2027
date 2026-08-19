#include "vex.h"

using namespace vex;

brain Brain;
controller primaryController(primary);
motor leftDriveFront(PORT1,ratio6_1,true);
motor leftDriveMiddle(PORT2,ratio6_1,false);
motor leftDriveRear(PORT3,ratio6_1,true);
motor rightDriveFront(PORT7,ratio6_1,false);
motor rightDriveMiddle(PORT8,ratio6_1,true);
motor rightDriveRear(PORT9,ratio6_1,false);
motor_group leftDrive(leftDriveFront,leftDriveMiddle,leftDriveRear);
motor_group rightDrive(rightDriveFront,rightDriveMiddle,rightDriveRear);
inertial imuSensor(PORT12);
motor intakeUpperMotor(PORT14,ratio6_1,true);
motor intakeLowerMotor(PORT11,ratio6_1,false);
digital_out ringRejectPiston(Brain.ThreeWirePort.A);
digital_out intakeClampPiston(Brain.ThreeWirePort.B);
digital_out scoringPiston(Brain.ThreeWirePort.G);
digital_out alignmentPiston(Brain.ThreeWirePort.H);

void vexcodeInit()
{
  leftDrive.setStopping(coast);rightDrive.setStopping(coast);
  intakeUpperMotor.setStopping(coast);intakeLowerMotor.setStopping(coast);
  resetPneumatics();
}
