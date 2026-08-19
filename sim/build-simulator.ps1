$ErrorActionPreference = "Stop"
$projectRoot = Split-Path $PSScriptRoot -Parent
$vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path -LiteralPath $vcvars)) {
  throw "Visual Studio C++ Build Tools not found."
}

$sources = @(
  "sim\mock\mock-vex.cpp",
  "sim\mock\mock-robot-config.cpp",
  "sim\mock\sim-main.cpp",
  "src\drive-control.cpp",
  "src\intake-control.cpp",
  "src\pneumatic-control.cpp",
  "src\driver-control.cpp",
  "src\tracking-odometry.cpp",
  "src\coordinate-drive.cpp",
  "src\autonomous-drive.cpp",
  "src\autonomous-routines.cpp",
  "src\auto-command-registry.cpp",
  "src\path-follower.cpp",
  "src\auto-runner.cpp",
  "src\auto-selector.cpp",
  "src\chassis-config.cpp",
  "src\JAR-Template\PID.cpp",
  "src\JAR-Template\util.cpp",
  "src\JAR-Template\odom.cpp",
  "src\JAR-Template\drive.cpp"
)
$quotedSources = ($sources | ForEach-Object { '"' + (Join-Path $projectRoot $_) + '"' }) -join " "
$outputDirectory = Join-Path $PSScriptRoot "bin"
New-Item -ItemType Directory -Force $outputDirectory | Out-Null
$outputFile = Join-Path $outputDirectory "vex-sim-core.exe"
$compile = "cl /nologo /std:c++17 /EHsc /D_USE_MATH_DEFINES /DVEX_DESKTOP_SIM /utf-8 /I`"$projectRoot\sim\mock`" /I`"$projectRoot\include`" $quotedSources /Fe:`"$outputFile`""
$commandLine = 'call "{0}" >nul && {1}' -f $vcvars, $compile
& cmd.exe /d /c $commandLine
if ($LASTEXITCODE -ne 0) { throw "VEX Desktop Simulator C++ build failed." }
Write-Host "Built: $outputFile"
