#include "vex.h"
#include "compiled-autos.h"

using namespace vex;

namespace
{
int selectedAutoIndex = -1;
bool autonomousTestRequested = false;

void drawSelector()
{
  Brain.Screen.setFillColor("#101820");
  Brain.Screen.setPenColor("#F2F7FA");
  Brain.Screen.clearScreen();
  Brain.Screen.printAt(18, 28, "AUTONOMOUS SELECT");
  Brain.Screen.printAt(18, 50, "Touch one program before the match");

  const std::size_t choiceCount = 1 + sizeof(compiledAutos::autos) / sizeof(compiledAutos::autos[0]);
  for (std::size_t choice = 0; choice < choiceCount && choice < 6; ++choice)
  {
    const int column = static_cast<int>(choice % 2);
    const int row = static_cast<int>(choice / 2);
    const int x = 18 + column * 234;
    const int y = 62 + row * 54;
    const bool selected = static_cast<int>(choice) - 1 == selectedAutoIndex;
    Brain.Screen.setFillColor(selected ? "#F2B84B" : "#22303C");
    Brain.Screen.setPenColor(selected ? "#101820" : "#8FA2B2");
    Brain.Screen.drawRectangle(x, y, 210, 44);
    const char *name = choice == 0 ? "NO AUTO" : compiledAutos::autos[choice - 1].name;
    Brain.Screen.setPenColor(selected ? "#101820" : "#F2F7FA");
    Brain.Screen.printAt(x + 12, y + 28, false, "%s", name);
  }

  Brain.Screen.setPenColor("#F2F7FA");
  Brain.Screen.printAt(18, 194, false, "SELECTED: %s",
    selectedAutoIndex >= 0 ? compiledAutos::autos[selectedAutoIndex].name : "NO AUTO");
  Brain.Screen.setPenColor(selectedAutoIndex >= 0 ? "#63E6A6" : "#55C2FF");
  Brain.Screen.printAt(18, 224, selectedAutoIndex >= 0
    ? "READY - autonomous may run"
    : "SAFE - autonomous will stay stopped");
  if (selectedAutoIndex >= 0)
  {
    Brain.Screen.setFillColor("#D64550");
    Brain.Screen.setPenColor("#FFFFFF");
    Brain.Screen.drawRectangle(346, 207, 116, 48);
    Brain.Screen.printAt(360, 237, "RUN TEST");
  }
}

void handleSelectorTouch()
{
  const int x = Brain.Screen.xPosition();
  const int y = Brain.Screen.yPosition();
  if (selectedAutoIndex >= 0 && x >= 346 && x <= 462 && y >= 207 && y <= 255)
  {
    autonomousTestRequested = true;
    return;
  }
  if (x < 18 || x > 462 || y < 62 || y > 224) return;
  const int column = x >= 252 ? 1 : 0;
  if ((column == 0 && x > 228) || (column == 1 && x < 252)) return;
  const int row = (y - 62) / 54;
  if (y > 62 + row * 54 + 44) return;
  const int choice = row * 2 + column;
  const int autoCount = static_cast<int>(sizeof(compiledAutos::autos) / sizeof(compiledAutos::autos[0]));
  if (choice > autoCount) return;
  selectedAutoIndex = choice - 1;
  drawSelector();
}
}

void initializeAutoSelector()
{
  // 安全預設：沒有人工選擇時不執行 Auto。
  selectedAutoIndex = -1;
  autonomousTestRequested = false;
  Brain.Screen.pressed(handleSelectorTouch);
  drawSelector();
}

bool isAutonomousEnabled()
{
  return selectedAutoIndex >= 0;
}

const char *selectedAutonomousName()
{
  return selectedAutoIndex >= 0 ? compiledAutos::autos[selectedAutoIndex].name : "none";
}

bool takeAutonomousTestRequest()
{
  const bool requested = autonomousTestRequested;
  autonomousTestRequested = false;
  return requested;
}
