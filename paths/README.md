# Path Planner files

模擬器的 `Path Planner／路徑編輯` 會將每條路徑存成 JSON 放在本資料夾，
並同步產生 `include/generated-path.h`，讓 VS Code 中的 VEX C++ 可以直接引用。

座標系統：左下角為 `(0, 0)`、單位為 inch、場地大小 `144 x 144 in`、
`headingDeg = 0` 朝向場地上方（`+Y`）。

底盤模式：

- `Tank Drive／坦克底盤`（預設）：車頭方向由路徑切線自動計算，Heading 不可獨立編輯。
- `Mecanum／麥克納姆`：移動方向與車頭方向可分離，Heading 可由每個 waypoint 獨立設定。

儲存後，JSON 的 `drivetrain` / `holonomic` 與 `include/generated-path.h` 的
`driveType` / `holonomic` 會保留這項差異，後續 PATH follower 必須依此選擇底盤運動模型。
