$ErrorActionPreference = "Stop"

$nodeCandidates = @(@(
  (Get-Command node -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source -ErrorAction SilentlyContinue),
  (Join-Path $env:USERPROFILE ".cache\codex-runtimes\codex-primary-runtime\dependencies\node\bin\node.exe")
) | Where-Object { $_ -and (Test-Path -LiteralPath $_) })

if ($nodeCandidates.Count -eq 0) {
  throw "Node.js was not found. Install the Node.js LTS release first."
}

$nodeExecutable = $nodeCandidates[0]
$simulatorUrl = "http://127.0.0.1:4173"

# Stop only the simulator already listening on its dedicated local port.
try {
  Invoke-WebRequest -UseBasicParsing -Method Post "$simulatorUrl/api/shutdown" -TimeoutSec 1 | Out-Null
  Start-Sleep -Milliseconds 350
} catch {
  $listener = Get-NetTCPConnection -LocalAddress "127.0.0.1" -LocalPort 4173 -State Listen -ErrorAction SilentlyContinue
  if ($listener) {
    $process = Get-Process -Id $listener.OwningProcess -ErrorAction SilentlyContinue
    if ($process -and $process.ProcessName -eq "node") {
      Stop-Process -Id $process.Id -Force
      Start-Sleep -Milliseconds 250
    } elseif ($process) {
      throw "Port 4173 is used by another program: $($process.ProcessName)."
    }
  }
}

# Rebuild so the simulator executes the current C++ sources.
& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "build-simulator.ps1")
if ($LASTEXITCODE -ne 0) { throw "C++ simulator core build failed." }

Start-Process -FilePath $nodeExecutable `
  -ArgumentList "server.mjs" `
  -WorkingDirectory $PSScriptRoot `
  -WindowStyle Hidden

$serverReady = $false
for ($attempt = 0; $attempt -lt 30; $attempt++) {
  Start-Sleep -Milliseconds 100
  try {
    $state = Invoke-WebRequest -UseBasicParsing "$simulatorUrl/api/state" -TimeoutSec 1
    if ($state.StatusCode -eq 200) { $serverReady = $true;break }
  } catch {}
}

if (-not $serverReady) {
  throw "Simulator server failed to start. Check the VS Code terminal output."
}

Start-Process $simulatorUrl
Write-Host "VEX Desktop Simulator started: $simulatorUrl"
