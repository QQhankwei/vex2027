# VEX V5 Competition Teaching Template

## Desktop Simulator

沒有 V5 Brain 時，可在 VS Code 執行工作 `VEX Simulator: Start`，使用本機
瀏覽器測試遙控配置、Intake、氣壓與 autonomous path。操作方式請見
[`GETTING_STARTED.md`](GETTING_STARTED.md)。

交給其他 Windows 使用者時，也可以直接雙擊專案根目錄的
`啟動模擬器.bat`。第一次使用前需安裝 Node.js LTS 與 Visual Studio 2022
Build Tools（Desktop development with C++）。重複啟動時，腳本會安全關閉
舊模擬核心、重新編譯目前 C++，再開啟新版頁面。

由 Team 88168A 競賽程式重新整理的教學骨架。原始版本曾在台灣賽事達到
Rank 2；新版保留 JAR-Template，並將硬體、子系統、遙控、自動程式與
座標追蹤分層，方便學生逐步理解與擴充。

第一次拿到本專案，請先依照 [`GETTING_STARTED.md`](GETTING_STARTED.md)
完成環境、硬體、遙控與短 Auto 測試。

## 架構

```text
比賽流程
└─ main.cpp
   ├─ preAutonomous()       開機與 IMU 校正
   ├─ autonomous()          選擇一個 auton routine
   └─ driverControl()       固定週期更新遙控功能

硬體配置
├─ robot-config.h/.cpp      Port、反轉、齒輪匣、三線輸出
└─ robot-parameters.h       單位、速度、尺寸、追蹤輪參數

可重用子系統
├─ drive-control            Tank Drive 與 deadband
├─ intake-control           Intake 狀態機
├─ pneumatic-control        氣動 set / toggle / reset
├─ driver-control           控制器映射與優先權
├─ autonomous-routines      自動程式組合入口
├─ autonomous-drive         Encoder / IMU / PID 閉迴路動作
├─ JAR-Template/PID         唯一且可重用的 PID 計算核心
└─ tracking-odometry        Tracking Wheels 座標計算核心

進階框架（保留，基礎版暫不編譯）
└─ JAR-Template
   ├─ PID
   ├─ Drive
   ├─ Odom
   └─ Util
```

## 控制器配置範例

| 輸入 | 功能 | 控制方式 |
|---|---|---|
| Axis3 | 左側底盤 | 持續讀取 |
| Axis2 | 右側底盤 | 持續讀取 |
| A | Upper Intake 正轉 | 按住運轉、放開停止 |
| B | Upper Intake 反轉 | 按住運轉、放開停止 |
| X | Intake 正轉 Toggle 範例 | 按一下運轉、再按一下停止 |
| R1 | Intake 正轉 | 按住 |
| R2 | Intake 反轉 | 按住 |
| L1 | 只有下層 Intake 正轉 | 按住 |
| L2 | 只有下層 Intake 反轉 | 按住 |
| Right | Intake Clamp 氣動 | 每按一次切換 |
| Up | Scoring 氣動 Hold 範例 | 按住 true、放開恢復 Toggle 狀態 |
| Y | Scoring 氣動 | 第一次 true、第二次 false |
| Down | Alignment 氣動 | 每按一次切換 |
| Left | Ring Reject 氣動 | 每按一次切換 |

持續動作使用 `pressing()` 放在固定週期迴圈；保持狀態的氣動功能使用
`pressed(callback)`，避免每個迴圈重複切換。

馬達也可以使用 `pressed(callback)` 做 Toggle。本範本以 A 的 Upper Hold
與 X 的上下 Intake Toggle 比較兩種控制方式。Hold 類按鍵具有較高優先權；
放開 Hold 後，程式會回到 X 鍵保存的 Toggle 狀態。

氣動使用相同設計：Up 暫時把 `scoringPiston` 設為 `true`，Y 保存長期的
Toggle 狀態。放開 Up 後不會盲目寫入 `false`，而是恢復 Y 先前保存的狀態。

## 硬體配置

| 名稱 | Port | 類型 | 說明 |
|---|---:|---|---|
| leftDriveFront | 1 | 6:1 Motor | 左前底盤 |
| leftDriveMiddle | 2 | 6:1 Motor | 左中底盤 |
| leftDriveRear | 3 | 6:1 Motor | 左後底盤 |
| rightDriveFront | 7 | 6:1 Motor | 右前底盤 |
| rightDriveMiddle | 8 | 6:1 Motor | 右中底盤 |
| rightDriveRear | 9 | 6:1 Motor | 右後底盤 |
| intakeLowerMotor | 11 | 6:1 Motor | 下層 Intake |
| imuSensor | 12 | Inertial | 方向回授 |
| intakeUpperMotor | 14 | 6:1 Motor | 上層 Intake |
| ringRejectPiston | A | Digital Out | 異色環排除 |
| intakeClampPiston | B | Digital Out | Intake 夾持 |
| scoringPiston | G | Digital Out | 得分機構 |
| alignmentPiston | H | Digital Out | 對位機構 |

此表沿用競賽版本的接線基底；正式下載前必須依實機逐項確認。

## 單位規範

- 距離：英吋，後綴 `_IN`
- 角度：度，後綴 `_DEG`
- 時間：毫秒，後綴 `_MS`
- 速度：百分比，後綴 `_PERCENT`
- 電壓：伏特，後綴 `_VOLT`
- 裝置與函式：`lowerCamelCase`
- 型別：`UpperCamelCase`
- 常數：`UPPER_SNAKE_CASE`

## Tracking Wheels

`TrackingOdometry` 本身不綁定 Port，只接受：

1. 前向追蹤輪累積距離（英吋）。
2. 側向追蹤輪累積距離（英吋）。
3. IMU 累積角度（度）。

實機接法範例位於
`examples/tracking-wheels-coordinate-example.cpp.disabled`。確認 Port、輪徑、
offset 和正方向後才能移入正式程式。

座標定義：

- 原點由 `reset()` 決定。
- X 正方向為場地右方。
- Y 正方向為機器人初始前方。
- 0 度代表朝向 +Y。

## 閉迴路 Auto 範例

`runClosedLoopAutonomousExample()` 示範完整路徑：

1. IMU 當前方向歸零。
2. Intake 正轉，同時用 Encoder 定距前進 24 in，IMU 維持 0 deg。
3. Intake Clamp 氣缸設為 `true`。
4. 使用 IMU PID 原地轉到絕對 90 deg。
5. 保持 90 deg 前進 12 in。
6. Alignment 與 Scoring 氣缸伸出、等待 300 ms、收回。
7. 保持 90 deg 後退 6 in。
8. 停止所有子系統並重設氣動。

每一段閉迴路動作都有 tolerance、settle time 與 timeout，避免單一 PID 永久
卡住。Auto 路徑採用線性動作清單寫法，方便學生閱讀和快速修改。

要啟用時，只在 `main.cpp` 的 `autonomous()` 保留：

```cpp
void autonomous()
{
  runClosedLoopAutonomousExample();
}
```

PID 計算與使用方式已恢復為原生 JAR-Template；預設常數沿用舊競賽程式的
`default_constants()`。硬體重量或齒比改變時仍需依實機重新驗證。

### JAR-Template PID 設定方式

`configureChassisConstants()` 會在開機時設定原生 JAR Drive、Heading、Turn、
Swing PID。一般路徑直接使用這組設定：

```cpp
driveDistanceIn(24.0, 0.0);
turnToHeadingDeg(90.0);
```

需要針對下一段動作調參時，沿用原本 JAR-Template 寫法：

```cpp
chassis.set_drive_constants(12, 0.5, 0.004, 5, 20);
chassis.set_heading_constants(12, 0.25, 0.007, 5, 40);
driveDistanceIn(12.0, 90.0);

chassis.set_turn_constants(8, 0.30, 0.001, 2.5, 60);
turnToHeadingDeg(90.0);
```

參數順序與原生 JAR 相同：`max_voltage, kp, ki, kd, starti`。設定會持續
套用到後續動作，直到再次呼叫 `set_*_constants()` 或重新執行預設設定。

## 座標移動

目前可直接使用：

```cpp
setRobotPose(0.0, 0.0, 0.0);
driveToPoint(0.0, 24.0);
turnToPoint(24.0, 36.0);
driveToPose(24.0, 36.0, 90.0);
```

現階段座標來源為底盤 Encoder＋IMU，側向距離視為 0，適合無 Tracking Wheel
的 Tank Drive。接上實體 Tracking Wheels 後，只需替換座標來源，不需要修改
Auto 路徑的函式名稱。

## Build

使用 VS Code 開啟整個專案資料夾，並安裝官方 `VEX Robotics` Extension。
工作區必須設為 Trusted。VEX Extension 下方狀態列出現 Build 圖示後即可編譯。

目前版本已使用 V5 SDK `V5_20240802_15_00_00` 編譯成功。

## 建議教學順序

1. `robot-config.cpp`：Port、反轉與齒輪匣。
2. `robot-parameters.h`：單位與集中參數。
3. `drive-control.cpp`：輸入、deadband、輸出。
4. `driver-control.cpp`：按住、按一下與功能優先權。
5. Intake 與 pneumatic 子系統。
6. `autonomous-routines.cpp`：用既有介面組合動作。
7. `tracking-odometry.cpp`：局部位移到場地座標。
8. 最後接回 JAR-Template 的 PID、定距、轉向與座標移動。
