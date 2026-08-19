# 教學講義索引

## 第一單元：硬體不是程式細節

先從 `robot-config.cpp` 找出每個實體裝置。學生必須能回答：

- 這個名稱對應機器上的哪一顆馬達？
- Port 是多少？
- 為什麼需要 reversed？
- 馬達 cartridge 是 6:1、18:1 還是 36:1？
- Digital Out 的 true 代表氣缸伸出還是縮回？

不要在其他檔案再次建立相同硬體物件。所有模組都透過
`robot-config.h` 的 `extern` 使用同一個實體。

## 第二單元：輸入、決策、輸出

以 Tank Drive 為例：

```text
Controller Axis
→ applyDeadband()
→ 限制 -100 至 100
→ motor_group.spin()
```

讓學生把每個子系統都畫成相同資料流，先理解責任，再開始改程式。

## 第三單元：按住與 Toggle

- Intake 必須「按住才運轉」，所以每 20 ms 使用 `pressing()` 讀取。
- 氣缸必須「按一次保持狀態」，所以使用 `pressed(callback)`。
- callback 只註冊一次，不能放在永久迴圈中。
- 同一按鍵不要同時負責兩個互相衝突的子系統。

本範本提供兩個最直接的比較：

```text
A 鍵：pressing() → 按住時 Upper Intake 正轉，放開停止
B 鍵：pressing() → 按住時 Upper Intake 反轉，放開停止
X 鍵：pressed()  → 第一次 Intake 持續正轉，第二次停止
Up鍵：pressing() → 氣壓按住 true，放開恢復原本狀態
Y 鍵：pressed()  → 第一次 scoringPiston=true，第二次=false
```

## 第四單元：Autonomous 是動作組合

Autonomous 不應直接散落硬體細節，而要組合：

- `setDrivePercent()`
- `stopDrive()`
- `setIntakeMode()`
- `setIntakeClamp()`

第一個範例是依時間控制的開迴路。它容易理解，但距離會受電量、摩擦與
碰撞影響。`runClosedLoopAutonomousExample()` 則展示 Encoder 定距、IMU
保角、PID 轉向、timeout 中止以及 Intake／氣動整合。

## 第五單元：Tracking Wheels 與座標

Rotation Sensor 先換算成英吋，再交給 `TrackingOdometry::update()`。
座標計算分成：

1. 求這一週期的前向、側向與角度變化。
2. 扣除機器人轉彎造成的追蹤輪圓弧位移。
3. 用平均 heading 將局部位移旋轉到場地座標。
4. 累加成 X、Y、heading。

一定要先量測 wheel diameter 與 signed offset。沒有量測就不能把座標誤差
歸因於公式或 PID。

## 第六單元：接回 JAR-Template

JAR-Template 目前保留但不參與基礎版編譯。學生理解上述模組後，再依序接回：

1. `util`：角度與限幅工具。
2. `PID`：P、I、D 與 settle / timeout。
3. `drive`：定距、轉向與 swing。
4. `odom`：座標控制。

接回時應將 JAR-Template 的舊全域名稱改成新版硬體介面，不能同時維護兩套
`chassis` 或兩套里程計 task。
