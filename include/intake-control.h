#pragma once

/** Intake 可讀的工作狀態，讓 driver control 與 autonomous 共用同一介面。 */
enum class IntakeMode
{
  Stopped,
  Forward,
  Reverse,
  UpperForward,
  UpperReverse,
  LowerForward,
  LowerReverse
};

/**
 * 設定 Intake 工作狀態。
 *
 * Forward / Reverse 會同時控制上下兩顆馬達；UpperForward / UpperReverse
 * 只控制上層；LowerForward / LowerReverse 只控制下層，方便分段測試、
 * 手動送環或解除卡料。
 */
void setIntakeMode(IntakeMode mode);

/** 停止兩顆 Intake 馬達。 */
void stopIntake();

/**
 * 切換 Intake 的持續運轉狀態。
 * 第一次呼叫後持續正轉，第二次呼叫後停止，適合綁定 Button.pressed()。
 */
void toggleIntakeForward();

/** 回傳 Toggle 模式目前是否要求 Intake 持續運轉。 */
bool isIntakeToggleRunning();

/** 清除 Toggle 狀態，模式切換或緊急停止時使用。 */
void resetIntakeToggle();
