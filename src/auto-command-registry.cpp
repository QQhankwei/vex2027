#include "vex.h"
#include "auto-command-registry.h"

#include <cstring>

bool executeAutoCommand(AutoCommandId commandId)
{
  switch (commandId)
  {
#define AUTO_COMMAND(symbol, slug, english, chinese, action) \
    case AutoCommandId::symbol: action; return true;
#include "auto-commands.def"
#undef AUTO_COMMAND
  }

  return false;
}

bool executeNamedAutoCommand(const char *commandName)
{
  if (commandName == nullptr)
  {
    return false;
  }

#define AUTO_COMMAND(symbol, slug, english, chinese, action) \
  if (std::strcmp(commandName, slug) == 0)                 \
  {                                                        \
    return executeAutoCommand(AutoCommandId::symbol);       \
  }
#include "auto-commands.def"
#undef AUTO_COMMAND

  return false;
}
