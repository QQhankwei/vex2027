#pragma once

enum class AutoRunResult
{
  Completed,
  UnknownAuto,
  PathFailed,
  UnknownCommand
};

/** 依名稱執行 Auto Studio 已編譯登錄表中的自動流程。 */
AutoRunResult runAuto(const char *autoName);
