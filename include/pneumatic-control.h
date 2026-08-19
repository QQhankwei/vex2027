#pragma once

/** 氣動機構的集中控制介面。 */
void setRingReject(bool active);
void setIntakeClamp(bool active);
void setScoringPiston(bool active);
void setAlignmentPiston(bool active);

/** Toggle 適合綁定控制器 pressed 事件：每按一次切換一次。 */
void toggleRingReject();
void toggleIntakeClamp();
void toggleScoringPiston();
void toggleAlignmentPiston();

/**
 * Scoring Piston 的 Hold 控制範例。
 * pressed=true 時暫時輸出 true；放開後恢復 Y 鍵保存的 Toggle 狀態。
 */
void updateScoringPistonHold(bool pressed);

/** 將所有氣動輸出回到 false，供開機或緊急停止使用。 */
void resetPneumatics();
