const { app, BrowserWindow, shell } = require("electron");
const { cpSync, existsSync, mkdirSync } = require("node:fs");
const { createServer } = require("node:net");
const { join } = require("node:path");
const { spawn } = require("node:child_process");

let mainWindow = null;
let simulatorProcess = null;
let simulatorPort = null;

function findAvailablePort() {
  return new Promise((resolve, reject) => {
    const probe = createServer();
    probe.once("error", reject);
    probe.listen(0, "127.0.0.1", () => {
      const address = probe.address();
      probe.close(() => resolve(address.port));
    });
  });
}

function copyDirectoryOnce(source, destination) {
  if (existsSync(destination)) return;
  mkdirSync(destination, { recursive: true });
  cpSync(source, destination, { recursive: true, force: false });
}

function runtimeLocations() {
  if (!app.isPackaged) {
    const projectRoot = join(__dirname, "..");
    return {
      simulatorDirectory: join(projectRoot, "sim"),
      workspaceDirectory: projectRoot
    };
  }

  const bundledRuntime = join(process.resourcesPath, "runtime");
  const workspaceDirectory = join(app.getPath("userData"), "workspace");
  copyDirectoryOnce(join(bundledRuntime, "workspace", "autos"),
                    join(workspaceDirectory, "autos"));
  copyDirectoryOnce(join(bundledRuntime, "workspace", "paths"),
                    join(workspaceDirectory, "paths"));
  copyDirectoryOnce(join(bundledRuntime, "workspace", "include"),
                    join(workspaceDirectory, "include"));
  return {
    simulatorDirectory: join(bundledRuntime, "sim"),
    workspaceDirectory
  };
}

function waitForSimulator(url, attempts = 80) {
  return new Promise((resolve, reject) => {
    let remaining = attempts;
    const check = () => {
      const request = require("node:http").get(`${url}/api/state`, response => {
        response.resume();
        if (response.statusCode === 200) return resolve();
        retry();
      });
      request.on("error", retry);
      request.setTimeout(500, () => request.destroy());
    };
    const retry = () => {
      if (--remaining <= 0) return reject(new Error("Simulator startup timed out."));
      setTimeout(check, 100);
    };
    check();
  });
}

async function startSimulator() {
  const locations = runtimeLocations();
  simulatorPort = await findAvailablePort();
  const serverPath = join(locations.simulatorDirectory, "server.mjs");
  simulatorProcess = spawn(process.execPath, [serverPath], {
    cwd: locations.simulatorDirectory,
    windowsHide: true,
    stdio: ["ignore", "pipe", "pipe"],
    env: {
      ...process.env,
      ELECTRON_RUN_AS_NODE: "1",
      VEX_SIM_PORT: String(simulatorPort),
      VEX_PROJECT_DIR: locations.workspaceDirectory
    }
  });
  simulatorProcess.stdout.on("data", data => console.log(String(data).trim()));
  simulatorProcess.stderr.on("data", data => console.error(String(data).trim()));
  simulatorProcess.once("exit", code => {
    if (!app.isQuitting && code !== 0)
      console.error(`RIFTIORY simulator stopped with code ${code}.`);
  });
  const url = `http://127.0.0.1:${simulatorPort}`;
  await waitForSimulator(url);
  return url;
}

async function createMainWindow() {
  const simulatorUrl = await startSimulator();
  mainWindow = new BrowserWindow({
    width: 1500,
    height: 940,
    minWidth: 1100,
    minHeight: 720,
    show: false,
    backgroundColor: "#070a0f",
    autoHideMenuBar: true,
    title: "RIFTIORY Robot Operations Console",
    webPreferences: {
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true
    }
  });
  mainWindow.webContents.setWindowOpenHandler(({ url }) => {
    if (/^https?:\/\//i.test(url)) shell.openExternal(url);
    return { action: "deny" };
  });
  mainWindow.once("ready-to-show", () => {
    mainWindow.maximize();
    mainWindow.show();
  });
  await mainWindow.loadURL(simulatorUrl);
}

function stopSimulator() {
  if (!simulatorProcess || simulatorProcess.killed) return;
  simulatorProcess.kill();
  simulatorProcess = null;
}

const singleInstanceLock = app.requestSingleInstanceLock();
if (!singleInstanceLock) app.quit();
else {
  app.on("second-instance", () => {
    if (!mainWindow) return;
    if (mainWindow.isMinimized()) mainWindow.restore();
    mainWindow.focus();
  });
  app.whenReady().then(createMainWindow).catch(error => {
    console.error(error);
    app.quit();
  });
  app.on("window-all-closed", () => app.quit());
  app.on("before-quit", () => {
    app.isQuitting = true;
    stopSimulator();
  });
}

