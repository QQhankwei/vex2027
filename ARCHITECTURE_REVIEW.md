# VEX 機器人程式架構複習與教學整理

> 專案：Team 88168A VEX V5 C++
> 盤點日期：2026-08-18
> 用途：程式複習、交接與教學準備

## 1. 版本與年份說明

- 本專案是 2024 賽季期間累積的程式，於 2025 年才上傳／交接到 Git。
- 目前 Git 可查到的紀錄為 2025-09-02 至 2025-09-07，共 7 次提交；提交日期代表上傳與交接時間，不等同實際開發起始日期。
- 目前工作目錄沒有 2026 年的程式提交，因此本文件以目前保存的最終版本進行架構複習，不強行推定逐年演進內容。
- README 中的部分硬體表格、函式名稱與目前程式不完全一致，教學時應以 `src/robot-config.cpp` 和實際機器接線為準。
- 目前程式看起來是由較早的賽季專案延續修改而成，底層使用 JAR-Template，上層自動路徑已改成新的 2025 版本，但仍保留一些舊函式與測試程式。

## 2. 一句話理解整個專案

這是一套以 VEX Competition Framework 為生命週期核心、JAR-Template 為底盤控制核心，再加上隊伍自訂的自動路徑、手動控制、手臂、進料、辨色與 Brain 儀表板的 C++ 專案。

## 3. 整體分層

```text
比賽框架與入口
└─ src/main.cpp
   ├─ pre_auton()       賽前初始化、自動路徑選擇、狀態頁
   ├─ autonomous()      依選擇執行自動路徑
   └─ usercontrol()     啟動各子系統 task，持續執行手動底盤控制

隊伍功能層
├─ src/autons.cpp       自動路徑、PID 預設參數、測試路徑
├─ src/arm.cpp          手臂／吊掛的手動 task
├─ src/autoarm.cpp      自動階段的手臂動作
└─ src/note.cpp         光學感測器辨色及排除異色物件

硬體抽象層
└─ src/robot-config.cpp 馬達、感測器、氣動元件及連接埠實體

共用控制框架（JAR-Template）
├─ drive.cpp / drive.h  底盤、轉向、定距、座標移動、手動控制
├─ PID.cpp / PID.h      PID 計算及結束條件
├─ odom.cpp / odom.h    里程計座標更新
└─ util.cpp / util.h    角度、電壓、限幅、deadband 等工具
```

## 4. 程式啟動與執行流程

```text
main()
├─ 註冊 Competition.autonomous(autonomous)
├─ 註冊 Competition.drivercontrol(usercontrol)
├─ 呼叫 pre_auton()
│  ├─ vexcodeInit()
│  ├─ default_constants()
│  ├─ 校正 Inertial
│  └─ 顯示 10 個自動路徑選項與狀態 Dashboard
└─ 進入永久等待迴圈

比賽進入 Autonomous
└─ autonomous()
   ├─ 設定聯盟顏色
   └─ switch(current_auton_selection) 執行對應路徑

比賽進入 Driver Control
└─ usercontrol()
   ├─ 啟動 autonoteTask()
   ├─ 啟動 momogoTask()
   ├─ 啟動 intakeControlTask()
   ├─ 啟動 hangControlTask()
   ├─ 綁定控制器按鍵 callback
   └─ 永久執行 chassis.control_tank(100)
```

## 5. 各檔案教學重點

### `src/main.cpp`：總控與使用者介面

主要責任：

- 建立全域 `Drive chassis`，設定底盤形式、馬達群組、輪徑、齒比與 IMU。
- 實作自訂的 `cos_move_distance_smooth()`，使用 cosine 速度曲線加減速，並用 IMU 修正方向。
- 實作氣動元件開關函式與控制器事件。
- 繪製 Brain 自動路徑選單和 Inputs／Motors 狀態頁。
- 管理 `pre_auton()`、`autonomous()`、`usercontrol()` 和 `main()`。

教學定位：先用此檔說明「比賽生命週期」，暫時不要一開始深入 Dashboard 畫圖細節。

### `src/robot-config.cpp`：硬體對照表

目前程式中的硬體定義：

| 類別 | 名稱 | Port / Three Wire | 備註 |
|---|---|---:|---|
| 左底盤馬達 | L1 / L2 / L3 | 1 / 2 / 3 | 6:1，反轉狀態不同 |
| 右底盤馬達 | R1 / R2 / R3 | 7 / 8 / 9 | 6:1，反轉狀態不同 |
| 慣性感測器 | Inertial | 12 | 底盤方向回授 |
| 上層進料 | intake | 14 | 6:1 |
| 光學辨色 | Optical | 7 | 與 R1 同為 Port 7，屬明顯衝突，必須核對實機 |
| 移動目標感測 | Optical_go | 16 | 接近偵測 |
| 下層進料 | intakedown | 11 | 6:1 |
| 手臂／吊掛 | hang1 | 19 | 36:1 |
| Vision LED | Vision1 / Vision2 | 13 / 15 | 此處主要當狀態燈使用 |
| 氣動輸出 | pushCylinder | A | 異色物件排除 |
| 氣動輸出 | intakeCylander | B | 進料機構 |
| 數位輸出 | redlight / whitelight | C / D | 狀態燈 |
| 氣動輸出 | shooter / aligner | G / H | 發射／對位機構 |

教學定位：先畫實體機器，再逐一對照名稱、Port、反轉方向與齒輪比。

### `src/autons.cpp`：自動路徑腳本

主要責任：

- 定義紅、藍聯盟共 10 個選單函式。
- 呼叫 `turn_to_angle()`、`drive_distance()` 與 `cos_move_distance_smooth()` 組合動作。
- 在路徑中直接控制 intake、intakedown、氣動元件與手臂。
- 集中設定預設 PID 與結束條件。
- 保留底盤、轉向、swing、odom 的測試函式。

現況：只有 `R_right()` 和 `B_right()` 有主要內容，其餘多數選單函式仍是空殼；`B_17022A()` 有完整舊路徑，但沒有被目前 `autonomous()` 的 switch 呼叫。

### `src/JAR-Template/drive.cpp`：底盤控制核心

主要能力：

- `drive_with_voltage()`：直接輸出左右電壓。
- `turn_to_angle()`：使用 IMU 與 PID 原地轉向。
- `drive_distance()`：使用馬達位置控制距離，並用 heading PID 保持方向。
- `left_swing_to_angle()`／`right_swing_to_angle()`：固定一側的 swing turn。
- `drive_to_point()`／`turn_to_point()`：配合 odometry 做座標控制。
- `control_tank()`：左右搖桿分別控制左右底盤。

教學定位：這是控制理論的核心，建議在學生先理解感測器回授、error 與 voltage output 後再講。

### `src/JAR-Template/PID.cpp`：回授控制器

核心公式：

```text
output = kp × error
       + ki × accumulated_error
       + kd × (error - previous_error)
```

- P：現在離目標多遠。
- I：長時間累積的小誤差。
- D：誤差變化速度，用來抑制震盪。
- `starti`：只在誤差小於此值時累加 I。
- `is_settled()`：誤差維持在容許範圍，或超過 timeout，就結束動作。

### `src/JAR-Template/odom.cpp`：位置追蹤

- 讀取前向與側向 tracker 的位移差。
- 結合前後兩次角度，換算機器人的局部位移。
- 將局部位移旋轉到場地座標，累加至 X、Y。
- 目前 `Drive` 設定為 `ZERO_TRACKER_ODOM`，前向距離實際來自右側底盤馬達，側向位移固定為 0。

### `src/arm.cpp`：手臂／吊掛

- 以永久 task 讀取控制器按鍵。
- L1、A、R1+R2／X、B 分別執行不同方向、扭力或定位動作。
- 未按鍵時使用 hold，維持手臂位置。

### `src/note.cpp`：顏色排除

- 光學感測器偵測到近物體後讀取顏色。
- 紅隊遇到藍色、藍隊遇到紅色時，短暫啟動 `pushCylinder` 排除物件。
- `selectedTeamColor` 由 `autonomous()` 根據自動路徑選項設定。

## 6. 目前資料流

| 輸入 | 判斷／控制 | 輸出 |
|---|---|---|
| Controller Axis 3 / 2 | deadband、百分比、電壓換算 | 左右底盤馬達 |
| Inertial heading | heading error、PID 或 PI 修正 | 左右底盤差速電壓 |
| 馬達 encoder position | 角度換算英吋、距離 error | 定距移動停止條件 |
| Optical color | 聯盟色與偵測色比較 | pushCylinder |
| Optical_go near object | 接近狀態與 airspace | Vision LED 顏色／閃爍 |
| Controller buttons | callback 或 task 內優先順序 | intake、手臂、氣動元件 |
| Brain touch | 5 × 2 選單座標 | current_auton_selection |

## 7. 教學前必須說明的風險與不一致

以下先列為教學與實機驗證重點，尚未修改程式：

1. `R1` 和 `Optical` 同時使用 Smart Port 7，硬體配置衝突。
2. README 的 Port 表與 `robot-config.cpp` 不一致，不能直接當成現況接線表。
3. `Jar-Template.v5code` 內的 robotconfig 仍是更舊的硬體配置，也與程式不一致。
4. `Drive chassis` 建構子註解／參數中有多組輪徑與齒比；`cos_move_distance_smooth()` 又使用另一組 `3.25` 與 `0.75`，距離換算基準不一致。
5. `kMotors` 清單把 `L3` 放了兩次，缺少 `R3`，Dashboard 會把 R3 的資料顯示錯。
6. `autons.cpp` 內各函式宣告了區域 `selectedTeamColor`，會遮蔽全域變數，實際不會更新 `note.cpp` 使用的顏色。
7. 多數自動選項是空函式，但 UI 仍提供 10 個可選項，容易在比賽時誤選空路徑。
8. `usercontrol()` 的永久迴圈內沒有 wait；迴圈後的 `wait(20, msec)` 永遠不會執行，可能造成 CPU 不必要的忙碌。
9. `momogoTask()` 和 `autonoteTask()` 的迴圈正常路徑沒有固定 sleep；其中辨色排除還會因 0.3 秒 wait 阻塞該 task。
10. R1 同時被 intake task 使用，也被註冊為 shooter／aligner callback，功能互相耦合，教學與操作都容易混淆。
11. `intakeControlTask()` 中 R1 與 R2 的程式內容目前完全相同，註解所稱方向差異不存在。
12. `holonomic_drive_to_point()` 的 `clamp(drive_output, drive_max_voltage, drive_max_voltage)` 上下限相同，若未來啟用全向底盤會得到固定正輸出。
13. `Drive` 類別和 `main.cpp` 同時存在兩套定距控制方法，參數與停止邏輯不同，需明確區分使用情境。
14. 自動路徑中大量使用 magic number，缺少動作名稱與場地目標說明，不利教學、調參與除錯。

## 8. 建議教學順序（8 個單元）

1. **機器與程式對照**：馬達、感測器、氣動、Port、正反轉。
2. **VEX Competition 生命週期**：`main → pre_auton → autonomous/usercontrol`。
3. **手動控制**：搖桿、deadband、百分比到 12V、按鍵 callback 與 task。
4. **自動動作積木**：直走、轉彎、等待、進料、氣動控制。
5. **閉迴路與 PID**：目標、回授、誤差、輸出、settle、timeout。
6. **平滑移動**：cosine 加速／巡航／減速以及 IMU 保角。
7. **Odometry**：局部座標、全域座標、tracker 與 IMU。
8. **整合與除錯**：建立一條短自動路徑、觀察 Dashboard、逐段測試。

## 9. 建議的課堂實作路線

```text
確認單顆馬達方向
→ 確認左右底盤群組
→ 手動 tank drive
→ 開迴路定時直走
→ encoder 定距
→ IMU 轉向
→ PID 定距與轉向
→ 組合短自動路徑
→ 加入 intake／氣動
→ 最後才加入辨色與 odometry
```

每個新功能都應先單獨測試，再整合到完整自動路徑；不要一開始就用長路徑判斷單一模組是否正確。

## 10. 建議後續整理階段

### 階段 A：建立可信的現況基準

- 依實機重新確認所有 Port、齒輪比、反轉方向、輪徑與外部齒比。
- 確認 10 個 auton 選項哪些要保留、哪些尚未完成。
- 將 README、VEX 專案設定與 `robot-config.cpp` 統一。

### 階段 B：只做低風險整理

- 將 Dashboard 從 `main.cpp` 移至獨立模組。
- 將氣動 callback、進料 task、移動目標提示 task 分到子系統檔案。
- 集中管理底盤與 PID 常數，命名每段自動動作的目的。
- 補上 task 的固定更新週期與必要註解。

### 階段 C：建立教學版文件與範例

- 硬體接線表與機器圖。
- 控制器按鍵表。
- 最小範例：手動底盤、定距、轉向、PID、自動路徑。
- 調參紀錄表與常見問題排查表。

在完成實機核對前，不建議直接重構底盤與自動程式，否則可能把現有的硬體補償或賽場調參誤當成錯誤移除。
