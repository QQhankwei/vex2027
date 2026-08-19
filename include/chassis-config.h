#pragma once

class Drive;

/** 全專案唯一的 JAR-Template Drive 實體。 */
extern Drive chassis;

/** 集中設定 JAR Drive、Heading、Turn、Swing PID 與結束條件。 */
void configureChassisConstants();
