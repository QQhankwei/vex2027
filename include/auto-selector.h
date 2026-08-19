#pragma once

/** 初始化 V5 Brain 觸控式 Auto 選擇畫面。 */
void initializeAutoSelector();

/** 是否允許 autonomous() 執行目前已編譯的 Auto。 */
bool isAutonomousEnabled();

/** 目前選擇名稱；停用時回傳 "none"。 */
const char *selectedAutonomousName();

/** 取得並清除 Brain 的 RUN TEST 請求。 */
bool takeAutonomousTestRequest();
