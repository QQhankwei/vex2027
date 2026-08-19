#pragma once

/** Auto Builder 與機器人程式共用的 Named Command ID。 */
enum class AutoCommandId
{
#define AUTO_COMMAND(symbol, slug, english, chinese, action) symbol,
#include "auto-commands.def"
#undef AUTO_COMMAND
};

/** 使用強型別 ID 執行已註冊動作；成功執行時回傳 true。 */
bool executeAutoCommand(AutoCommandId commandId);

/** 使用 Auto JSON 保存的字串 ID 執行動作；找不到名稱時回傳 false。 */
bool executeNamedAutoCommand(const char *commandName);
