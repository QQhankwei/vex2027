#pragma once

#include <cmath>
#ifdef VEX_DESKTOP_SIM
// 桌面模擬器只替換 VEX 硬體層；控制、Auto 與 JAR PID 仍使用同一份原始碼。
#include "../sim/mock/v5.h"
#include "../sim/mock/v5_vcs.h"
#else
#include "v5.h"
#include "v5_vcs.h"
#endif

// 原生 JAR-Template Header 使用未加 vex:: 前綴的 VEX 型別。
// 為保持其原始 API 與相容性，必須在載入 JAR Header 前引入 vex namespace。
using namespace vex;

#include "robot-config.h"
#include "robot-parameters.h"
#include "drive-control.h"
#include "intake-control.h"
#include "pneumatic-control.h"
#include "driver-control.h"
#include "tracking-odometry.h"
#include "telemetry.h"
#include "autonomous-routines.h"
#include "JAR-Template/util.h"
#include "JAR-Template/PID.h"
#include "JAR-Template/odom.h"
#include "JAR-Template/drive.h"
#include "chassis-config.h"
#include "autonomous-drive.h"
#include "coordinate-drive.h"
#include "auto-command-registry.h"
#include "path-follower.h"
#include "auto-runner.h"
#include "auto-selector.h"
