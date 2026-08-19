#pragma once

/** 註冊「按一下切換一次」的控制器事件。只能在每次 driver control 開始時呼叫一次。 */
void configureDriverButtonCallbacks();

/** 讀取搖桿與需要持續按住的按鍵，更新所有手動子系統。 */
void updateDriverControls();

/** 停止馬達並重設氣動，供模式切換或測試結束使用。 */
void stopAllSubsystems();
