#pragma once

#include "v5.h"
#include "v5_vcs.h"

extern vex::brain Brain;
extern vex::controller primaryController;

extern vex::motor leftDriveFront;
extern vex::motor leftDriveMiddle;
extern vex::motor leftDriveRear;
extern vex::motor rightDriveFront;
extern vex::motor rightDriveMiddle;
extern vex::motor rightDriveRear;
extern vex::motor_group leftDrive;
extern vex::motor_group rightDrive;
extern vex::inertial imuSensor;

// Intake：上層負責輸送，下層負責接取／送入。
extern vex::motor intakeUpperMotor;
extern vex::motor intakeLowerMotor;

// 氣動輸出：名稱描述機構用途，不再使用 A、B、G、H 當程式名稱。
extern vex::digital_out ringRejectPiston;
extern vex::digital_out intakeClampPiston;
extern vex::digital_out scoringPiston;
extern vex::digital_out alignmentPiston;

void vexcodeInit();
