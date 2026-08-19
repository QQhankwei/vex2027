#pragma once

/** 將搖桿中央的小幅輸入視為 0；輸入、輸出單位皆為百分比。 */
int applyDeadband(int inputPercent);

/** 控制左右底盤；輸入單位為百分比，範圍 -100 至 100。 */
void setDrivePercent(int leftPercent, int rightPercent);

/** 停止左右底盤。 */
void stopDrive();
