#include "vex.h"
#include "compiled-autos.h"

#include <cstring>

AutoRunResult runAuto(const char *autoName)
{
  const compiledAutos::AutoDefinition *selected = nullptr;
  if (autoName != nullptr)
    for (const auto& candidate : compiledAutos::autos)
      if (std::strcmp(autoName, candidate.name) == 0) { selected = &candidate; break; }
  if (selected == nullptr)
  {
    stopAllSubsystems();
    return AutoRunResult::UnknownAuto;
  }

  for (std::size_t index = 0; index < selected->stepCount; ++index)
  {
    const auto& step = selected->steps[index];
    switch (step.type)
    {
      case compiledAutos::AutoStepType::FollowPath:
        if (followPath(step.target) != PathFollowResult::Completed)
        {
          stopAllSubsystems();
          return AutoRunResult::PathFailed;
        }
        break;
      case compiledAutos::AutoStepType::Wait:
        wait(static_cast<int>(step.value * 1000.0), msec);
        break;
      case compiledAutos::AutoStepType::NamedCommand:
        if (!executeNamedAutoCommand(step.target))
        {
          stopAllSubsystems();
          return AutoRunResult::UnknownCommand;
        }
        break;
    }
  }
  stopDrive();
  return AutoRunResult::Completed;
}
