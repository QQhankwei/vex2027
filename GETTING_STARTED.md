# 新人開始指南

## 沒有機器時：啟動 Desktop Simulator

1. 在 VS Code 按 `Ctrl + Shift + P`。
2. 執行 `Tasks: Run Task`。
3. 選擇 `VEX Simulator: Start`。
4. 瀏覽器會開啟 `http://127.0.0.1:4173`。

模擬器包含 `Teleoperated／遙控測試` 與 `Autonomous PATH／自動路徑`。
綠色 Intake 線代表馬達運轉，黃色伸縮線代表氣缸伸出。這是控制流程與
簡化運動模型，不包含輪胎打滑、負載、電池壓降或場地碰撞，上場前仍須實機驗證。

### 使用 USB Gamepad

1. 將 Xbox 或 Windows 可辨識的一般 USB Gamepad 接上電腦。
2. 保持模擬器分頁在最前面，按一下搖桿任意按鍵。
3. 右上角顯示 `GAMEPAD／搖桿已連線` 後即可操作。

標準配置：左／右搖桿控制底盤，A/B 控制 Upper Intake，X/Y 為 Toggle，
LB/LT 控制 Lower Intake，RB/RT 控制上下 Intake，方向鍵控制氣壓。
VEX 原廠 Controller 必須先被 Windows 辨識為標準 Gamepad 才能使用此功能。

羅技不同型號或 `XInput／DirectInput` 模式可能使用不同的瀏覽器軸編號。
若 VEX Axis2 沒有反應，先移動右搖桿並查看 `Raw axes／原始軸`，再從
`Right stick source／右搖桿來源` 選擇有變化的 Browser Axis 2、3 或 5。

`Drive motor speed／底盤馬達速度` 可設定 100–600 RPM。模擬器會搭配
3.75 英吋輪徑與 0.66666 外部齒比換算線速度；場地外牆與九個 Goal 都有
簡化碰撞範圍，碰撞時會停止平移，但仍允許轉向脫離。

這份文件提供給第一次拿到本專案的人。請依照順序操作，不要一開始就修改
PID、Tracking Wheels 或整條 Auto。

## 1. 先認識這是什麼專案

這是 VEX V5 C++ 競賽機器人程式，使用：

- VS Code
- 官方 VEX Robotics Extension
- VEX V5 C++ SDK
- JAR-Template 的 PID、Drive、Odom 與工具函式

程式分成三個比賽階段：

```text
preAutonomous()  → 開機初始化、設定 JAR constants、校正 IMU
autonomous()     → 比賽自動階段
driverControl()  → 遙控階段
```

入口位於 `src/main.cpp`。

## 2. 第一次開啟專案

1. 安裝 VS Code。
2. 安裝官方 `VEX Robotics` Extension。
3. 使用 VS Code 的 **Open Folder** 開啟整個 `vex-2024-main` 資料夾。
4. 若上方出現 Restricted Mode，選擇 **Trust this workspace**。
5. 等待右下角 IntelliSense 與 VEX Extension 初始化完成。
6. 確認下方狀態列顯示 `VEX-Teaching-Template`。

不要只開啟單一 `.cpp` 檔；VEX Extension 必須從專案根目錄讀取：

```text
.vscode/vex_project_settings.json
makefile
include/
src/
vex/
```

## 3. 第一次編譯

點擊 VS Code 下方狀態列的 VEX Build 圖示。

成功時會看到：

```text
Build Finished: Exit Code 0
```

輸出檔案位於：

```text
build/VEX-Teaching-Template.bin
```

如果顯示找不到 Toolchain，讓 VEX Extension 下載官方 C++ Toolchain，再重新
Build。

## 4. 下載前先核對硬體

硬體集中定義於：

```text
src/robot-config.cpp
include/robot-config.h
```

目前設定：

| 裝置 | Port | 功能 |
|---|---:|---|
| leftDriveFront | 1 | 左前底盤 |
| leftDriveMiddle | 2 | 左中底盤 |
| leftDriveRear | 3 | 左後底盤 |
| rightDriveFront | 7 | 右前底盤 |
| rightDriveMiddle | 8 | 右中底盤 |
| rightDriveRear | 9 | 右後底盤 |
| intakeLowerMotor | 11 | 下層 Intake |
| imuSensor | 12 | 慣性感測器 |
| intakeUpperMotor | 14 | 上層 Intake |
| ringRejectPiston | A | 排環氣動 |
| intakeClampPiston | B | Intake 夾持氣動 |
| scoringPiston | G | 得分氣動 |
| alignmentPiston | H | 對位氣動 |

正式下載前必須確認：

- Port 與實機相同。
- 馬達 reversed 設定正確。
- 馬達 cartridge 是正確齒比。
- 氣缸 `true` 的實際方向是否安全。
- 輪徑與外部齒比是否正確。

## 5. 第一次上機測試

第一次測試必須架高底盤，讓輪子離地，並讓機器周圍沒有其他人。

建議順序：

1. 只接底盤，確認六顆馬達方向。
2. 確認左右搖桿方向。
3. 測試 Upper Intake。
4. 測試 Lower Intake。
5. 降低氣壓後逐一測試氣缸。
6. 最後才測試 Auto。

若同一側有某顆輪子反向，不要在遙控程式乘以 `-1`；應修改
`robot-config.cpp` 對應馬達的 reversed 設定。

## 6. 遙控操作

遙控判斷集中於：

```text
src/driver-control.cpp
```

| 按鍵 | 功能 | 模式 |
|---|---|---|
| Axis3 | 左側底盤 | 持續控制 |
| Axis2 | 右側底盤 | 持續控制 |
| A | Upper Intake 正轉 | 按住運轉、放開停止 |
| B | Upper Intake 反轉 | 按住運轉、放開停止 |
| L1 | Lower Intake 正轉 | 按住運轉、放開停止 |
| L2 | Lower Intake 反轉 | 按住運轉、放開停止 |
| R1 | Upper＋Lower 正轉 | 按住運轉、放開停止 |
| R2 | Upper＋Lower 反轉 | 按住運轉、放開停止 |
| X | Upper＋Lower 正轉 | 按一下運轉、再按一下停止 |
| Up | Scoring 氣動 | 按住時 true，放開恢復 Y 狀態 |
| Y | Scoring 氣動 | 按一下 true、再按一下 false |
| Right | Intake Clamp 氣動 | 按一下切換 |
| Down | Alignment 氣動 | 按一下切換 |
| Left | Ring Reject 氣動 | 按一下切換 |

### Hold 與 Toggle

Hold 使用 `pressing()`：

```cpp
if (primaryController.ButtonA.pressing())
{
  setIntakeMode(IntakeMode::UpperForward);
}
```

Toggle 使用 `pressed(callback)`：

```cpp
primaryController.ButtonY.pressed(toggleScoringPiston);
```

Callback 只能註冊一次，不要放進永久 `while` 迴圈。

## 7. Intake 與氣動怎麼使用

不要在 Auto 或遙控程式直接操作 Intake 馬達，請使用：

```cpp
setIntakeMode(IntakeMode::Forward);
setIntakeMode(IntakeMode::Reverse);
setIntakeMode(IntakeMode::UpperForward);
setIntakeMode(IntakeMode::UpperReverse);
setIntakeMode(IntakeMode::LowerForward);
setIntakeMode(IntakeMode::LowerReverse);
stopIntake();
```

氣動在 Auto 中使用明確的 `true / false`：

```cpp
setIntakeClamp(true);
wait(250, msec);
setIntakeClamp(false);

setAlignmentPiston(true);
setScoringPiston(true);
wait(300, msec);
setScoringPiston(false);
setAlignmentPiston(false);
```

Auto 不需要使用遙控器的 Toggle callback。

## 8. Auto 寫在哪裡

Auto 路徑位於：

```text
src/autonomous-routines.cpp
include/autonomous-routines.h
```

目前提供：

```cpp
runDoNothingAutonomous();
runTimedAutonomousExample();
runClosedLoopAutonomousExample();
runCoordinateAutonomousExample();
```

實際比賽執行哪一個，由 `src/main.cpp` 決定：

```cpp
void autonomous()
{
  runDoNothingAutonomous();
}
```

測試閉迴路範例時改成：

```cpp
void autonomous()
{
  runClosedLoopAutonomousExample();
}
```

同一時間只能呼叫一條 Auto，不要把多條範例一起執行。

## 9. JAR-Template PID

本專案的定距、保角與轉向使用原生 JAR-Template：

```text
include/JAR-Template/
src/JAR-Template/
```

預設設定集中於 `src/chassis-config.cpp`：

```cpp
chassis.set_drive_constants(12, 0.5, 0.004, 5, 20);
chassis.set_heading_constants(12, 0.25, 0.007, 5, 40);
chassis.set_turn_constants(12, 0.35, 0.001, 3, 90);
```

參數順序：

```text
max_voltage, kp, ki, kd, starti
```

Auto 中可以針對下一段重新設定：

```cpp
chassis.set_drive_constants(8, 0.5, 0.004, 5, 20);
chassis.set_heading_constants(8, 0.25, 0.007, 5, 40);

driveDistanceIn(24.0, 0.0);
```

常用動作：

```cpp
driveDistanceIn(24.0, 0.0);   // 前進 24 in，保持 0 deg
driveDistanceIn(-6.0, 90.0);  // 後退 6 in，保持 90 deg
turnToHeadingDeg(90.0);       // 原地轉到絕對 90 deg
```

`driveDistanceIn()` 與 `turnToHeadingDeg()` 只是容易閱讀的名稱；底層實際呼叫
`chassis.drive_distance()` 與 `chassis.turn_to_angle()`。

## 10. 座標移動

座標 API：

```cpp
setRobotPose(0.0, 0.0, 0.0);
driveToPoint(0.0, 24.0);
turnToPoint(24.0, 36.0);
driveToPose(24.0, 36.0, 90.0);
```

座標定義：

- X 正方向為場地右方。
- Y 正方向為機器人起始前方。
- 0 度朝向 +Y。
- 距離單位為英吋。

目前使用底盤 Encoder＋IMU 的 zero-tracker odometry。若要使用實體 Tracking
Wheels，先參考：

```text
examples/tracking-wheels-coordinate-example.cpp.disabled
```

在確認 Tracking Wheel Port、直徑、offset 和正方向前，不要直接啟用範例。

## 11. 如何新增一條 Auto

先在 `include/autonomous-routines.h` 宣告：

```cpp
void myFirstAutonomous();
```

再到 `src/autonomous-routines.cpp` 實作：

```cpp
void myFirstAutonomous()
{
  chassis.set_drive_constants(8, 0.5, 0.004, 5, 20);
  chassis.set_heading_constants(8, 0.25, 0.007, 5, 40);
  chassis.set_turn_constants(8, 0.35, 0.001, 3, 90);

  setRobotPose(0.0, 0.0, 0.0);
  resetPneumatics();

  setIntakeMode(IntakeMode::Forward);
  driveDistanceIn(24.0, 0.0);
  turnToHeadingDeg(90.0);

  setIntakeClamp(true);
  wait(250, msec);

  driveDistanceIn(-6.0, 90.0);
  stopAllSubsystems();
}
```

最後在 `main.cpp` 的 `autonomous()` 呼叫它。

## 12. 建議調試方式

每次只驗證一件事：

```text
馬達方向
→ Tank Drive
→ IMU 校正
→ turnToHeadingDeg(90)
→ driveDistanceIn(12, 0)
→ Intake
→ 單一氣缸
→ 短 Auto
→ 完整 Auto
→ 座標移動
```

不要直接用完整 Auto 判斷單一馬達、PID 或氣缸是否正確。

## 13. 常見問題

### Build 顯示 No VEX Project

- 確認使用 Open Folder 開啟專案根目錄。
- 確認 Workspace 已 Trust。
- 確認 `.vscode/vex_project_settings.json` 存在。

### Failed to save: file is newer

代表檔案在 VS Code 外部被更新。先選 Compare：

- `in file` 是硬碟版本。
- `in Visual Studio Code` 是尚未儲存的編輯器版本。

確認內容後再選擇保留哪一側，不要直接 Overwrite。

### 機器直走時偏向一側

依序確認：

1. 輪胎、軸承和機構阻力。
2. 馬達方向與齒輪比。
3. IMU 是否完成校正。
4. 輪徑與外部齒比。
5. 最後才調整 heading PID。

### Auto 動作過早結束或執行太久

檢查 JAR exit conditions：

```cpp
chassis.set_drive_exit_conditions(error, settleTimeMs, timeoutMs);
chassis.set_turn_exit_conditions(error, settleTimeMs, timeoutMs);
```

## 14. 新人第一週目標

新人不需要第一週理解整套 Odom。建議完成：

1. 能成功 Build 與下載。
2. 能說出每顆馬達的名稱與 Port。
3. 能修改一個控制器按鍵。
4. 能獨立控制 Upper／Lower Intake。
5. 能寫一條包含直走、轉向、Intake 和氣壓的短 Auto。
6. 能解釋 Drive PID、Heading PID 與 Turn PID 的責任差異。

完成以上內容後，再進入 Tracking Wheels、座標與完整競賽路徑。
# 無實機桌面模擬／Desktop simulation

最簡單的方式是雙擊專案根目錄的 `啟動模擬器.bat`。

在 VS Code 執行 `Terminal → Run Task → VEX Simulator: Start`。任務會先編譯
`sim/bin/vex-sim-core.exe`，再開啟 `http://127.0.0.1:4173/`。

模擬器直接編譯並執行現有的 `driver-control.cpp`、`autonomous-routines.cpp`
與 `src/JAR-Template`。網頁只傳送 Axis、按鍵及顯示遙測；場地碰撞與馬達模型
仍是簡化物理，不能取代實機調 PID。

## Path Planner／路徑編輯

切換到 `Path Planner／路徑編輯` 後，在場地點擊建立 waypoint，拖曳可調整
位置；右側可輸入 X/Y（inch）、Heading 與速度。座標以場地左下角為 `(0,0)`，
`0°` 朝向 `+Y`。

按下 `SAVE TO VS CODE／存入 VS Code` 會同時建立：

- `paths/<path-name>.json`：可再次匯入編輯器的完整路徑資料。
- `include/generated-path.h`：VEX C++ 可直接引用的 `generatedPath::points`。

目前完成的是 waypoint 建立與資料交換；真正的路徑追蹤控制器及 Auto 執行
會在下一階段接到這份共用資料。
