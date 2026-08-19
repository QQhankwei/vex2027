#pragma once

/**
 * 啟動實機遙測工作。
 *
 * 每 100 ms 將一筆以 @VEX_TELEMETRY 開頭的 JSON 傳到 USB Terminal。
 * Desktop Dashboard 只解析此前綴，因此一般除錯文字不會破壞資料流。
 */
void startTelemetry();

