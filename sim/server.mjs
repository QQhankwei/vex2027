import http from "node:http";
import { mkdir, readFile, readdir, writeFile } from "node:fs/promises";
import { extname, join, normalize } from "node:path";
import { fileURLToPath } from "node:url";
import { spawn } from "node:child_process";

const simDirectory = fileURLToPath(new URL("./", import.meta.url));
const projectDirectory = normalize(join(simDirectory, ".."));
const port = Number(process.env.VEX_SIM_PORT || 4173);
const corePath = join(simDirectory, "bin", "vex-sim-core.exe");
const core = spawn(corePath, [], { stdio: ["pipe", "pipe", "pipe"] });
const eventClients = new Set();
const pendingStates = [];
let latestState = null;
let bufferedOutput = "";
let lastDriveCommandAt = 0;
let currentDriveMode = "tank";
let selectedAutoName = "none";

core.stdout.setEncoding("utf8");
core.stdout.on("data", chunk => {
  bufferedOutput += chunk;
  const lines = bufferedOutput.split(/\r?\n/);
  bufferedOutput = lines.pop() || "";
  for (const line of lines) {
    if (!line.trim()) continue;
    try { latestState = JSON.parse(line); } catch { continue; }
    if (latestState.mode === "driver") {
      for (let index = pendingStates.length - 1; index >= 0; index--) {
        if (pendingStates[index].mode === "driver") pendingStates.splice(index, 1);
      }
    }
    pendingStates.push(latestState);
  }
});
core.stderr.on("data", chunk => console.error(`[VEX C++ core] ${chunk}`));
core.on("exit", code => console.error(`VEX C++ core stopped (${code}).`));
setInterval(() => {
  const state = pendingStates.shift();
  if (!state) return;
  const packet = `data: ${JSON.stringify(state)}\n\n`;
  for (const client of eventClients) client.write(packet);
}, 20);
const mimeTypes = {
  ".html": "text/html; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".css": "text/css; charset=utf-8"
};

function validatePathDocument(value) {
  if (!value || typeof value !== "object" || !Array.isArray(value.points) || value.points.length < 2 || value.points.length > 100) return "Path must contain 2 to 100 points.";
  for (const point of value.points) {
    if (![point.xIn, point.yIn, point.headingDeg, point.velocityPct].every(Number.isFinite)) return "Every point requires numeric xIn, yIn, headingDeg and velocityPct.";
    if (point.xIn < 0 || point.xIn > 144 || point.yIn < 0 || point.yIn > 144 || point.velocityPct < 0 || point.velocityPct > 100) return "Point values are outside the field or velocity range.";
  }
  return null;
}

function generatePathHeader(document) {
  const rows = document.points.map(point => `  PathPoint{${point.xIn.toFixed(3)}, ${point.yIn.toFixed(3)}, ${point.headingDeg.toFixed(2)}, ${point.velocityPct.toFixed(1)}, ${(Number(point.resolvedTangentDeg) || 0).toFixed(2)}, ${(Number(point.tangentStrength) || 100).toFixed(1)}}`).join(",\n");
  const robotWidth = Number(document.robot?.widthIn) || 18;
  const robotLength = Number(document.robot?.lengthIn) || 18;
  const driveType = document.drivetrain === "mecanum" ? "Mecanum" : "Tank";
  return `#pragma once\n\n#include <array>\n#include <cstddef>\n\n// 由 VEX Desktop Simulator Path Planner 自動產生，請回到地圖編輯後重新儲存。\nnamespace generatedPath\n{\nenum class DriveType\n{\n  Tank,\n  Mecanum\n};\n\nstruct PathPoint\n{\n  double xIn;\n  double yIn;\n  double headingDeg;\n  double velocityPct;\n  double tangentDeg;\n  double tangentStrengthPct;\n};\n\ninline constexpr DriveType driveType = DriveType::${driveType};\ninline constexpr bool holonomic = ${driveType === "Mecanum"};\ninline constexpr double robotWidthIn = ${robotWidth.toFixed(2)};\ninline constexpr double robotLengthIn = ${robotLength.toFixed(2)};\ninline constexpr std::array<PathPoint, ${document.points.length}> points{{\n${rows}\n}};\n}\n`;
}

async function readAutoCommandDefinitions() {
  const source = await readFile(join(projectDirectory, "include", "auto-commands.def"), "utf8");
  return [...source.matchAll(/AUTO_COMMAND\(\s*([A-Za-z0-9_]+)\s*,\s*"([a-z0-9_-]+)"\s*,\s*"([^"]+)"\s*,\s*"([^"]+)"\s*,\s*(.+)\)/g)].map(match => ({ symbol: match[1], id: match[2], english: match[3], chinese: match[4], cpp: `${match[5].trim()};` }));
}

function validateAutoDocument(value, namedCommandIds) {
  const validTypes = new Set(["path", "wait", ...namedCommandIds]);
  if (!value || typeof value !== "object" || !Array.isArray(value.commands) || value.commands.length > 100) return "Auto must contain no more than 100 commands.";
  for (const command of value.commands) {
    if (!command || !validTypes.has(command.type)) return "Auto contains an unsupported command.";
    if (command.type === "path" && !/^[a-z0-9_-]{1,40}$/i.test(String(command.pathName || ""))) return "Follow Path requires a valid path name.";
    if (command.type === "wait" && (!Number.isFinite(command.seconds) || command.seconds < 0 || command.seconds > 30)) return "Wait must be between 0 and 30 seconds.";
  }
  return null;
}

function generateAutoHeader(document) {
  const rows = document.commands.map(command => `  AutoStep{AutoStepType::${command.type === "path" ? "FollowPath" : command.type === "wait" ? "Wait" : "NamedCommand"}, "${command.pathName || (command.type !== "wait" ? command.type : "")}", ${(Number(command.seconds) || 0).toFixed(3)}}`).join(",\n");
  return `#pragma once\n\n#include <array>\n\n// 由 VEX Desktop Simulator Auto Builder 自動產生。\nnamespace generatedAuto\n{\nenum class AutoStepType\n{\n  FollowPath,\n  Wait,\n  NamedCommand\n};\n\nstruct AutoStep\n{\n  AutoStepType type;\n  const char *target;\n  double value;\n};\n\ninline constexpr char name[] = "${document.name}";\ninline constexpr std::array<AutoStep, ${document.commands.length}> steps{{\n${rows}\n}};\n}\n`;
}

const cppSymbol = value => String(value).replace(/[^a-z0-9_]+/gi, "_").replace(/^([0-9])/, "_$1");
async function regenerateCompiledAutosHeader() {
  const autoDirectory = join(projectDirectory, "autos"),pathDirectory = join(projectDirectory, "paths");
  const autoEntries = await readdir(autoDirectory, { withFileTypes: true });
  const pathEntries = await readdir(pathDirectory, { withFileTypes: true });
  const autos = await Promise.all(autoEntries.filter(entry => entry.isFile() && entry.name.endsWith(".auto.json")).map(async entry => JSON.parse(await readFile(join(autoDirectory, entry.name), "utf8"))));
  const paths = await Promise.all(pathEntries.filter(entry => entry.isFile() && entry.name.endsWith(".json")).map(async entry => JSON.parse(await readFile(join(pathDirectory, entry.name), "utf8"))));
  const pathNames = new Set(paths.map(path => path.name));
  for (const auto of autos) for (const command of auto.commands || []) if (command.type === "path" && !pathNames.has(command.pathName)) throw new Error(`Auto ${auto.name} references missing path ${command.pathName}.`);
  const pathArrays = paths.map(path => {
    const symbol=`path_${cppSymbol(path.name)}`,rows=path.points.map(point=>`  PathPoint{${Number(point.xIn).toFixed(3)}, ${Number(point.yIn).toFixed(3)}, ${Number(point.headingDeg).toFixed(2)}, ${Number(point.velocityPct).toFixed(1)}, ${(Number(point.resolvedTangentDeg ?? point.tangentDeg) || 0).toFixed(2)}, ${(Number(point.tangentStrength) || 100).toFixed(1)}}`).join(",\n");
    return `inline constexpr PathPoint ${symbol}[]{\n${rows}\n};`;
  }).join("\n\n");
  const autoArrays = autos.map(auto => {
    const symbol=`auto_${cppSymbol(auto.name)}`,rows=(auto.commands||[]).map(command=>`  AutoStep{AutoStepType::${command.type === "path" ? "FollowPath" : command.type === "wait" ? "Wait" : "NamedCommand"}, "${command.pathName || (command.type !== "wait" ? command.type : "")}", ${(Number(command.seconds) || 0).toFixed(3)}}`).join(",\n");
    return `inline constexpr AutoStep ${symbol}[]{\n${rows}\n};`;
  }).join("\n\n");
  const pathDefinitions=paths.map(path=>{const symbol=`path_${cppSymbol(path.name)}`;return `  PathDefinition{"${path.name}", DriveType::${path.drivetrain === "mecanum" ? "Mecanum" : "Tank"}, ${symbol}, sizeof(${symbol}) / sizeof(${symbol}[0])}`}).join(",\n");
  const autoDefinitions=autos.map(auto=>{const symbol=`auto_${cppSymbol(auto.name)}`;return `  AutoDefinition{"${auto.name}", ${symbol}, sizeof(${symbol}) / sizeof(${symbol}[0])}`}).join(",\n");
  const header=`#pragma once\n\n#include <cstddef>\n\n// Auto Studio 自動產生：所有已儲存的 Path 與 Auto 都會一起編譯。\n+namespace compiledAutos\n+{\n+enum class DriveType { Tank, Mecanum };\n+enum class AutoStepType { FollowPath, Wait, NamedCommand };\n+struct PathPoint { double xIn; double yIn; double headingDeg; double velocityPct; double tangentDeg; double tangentStrengthPct; };\n+struct PathDefinition { const char *name; DriveType driveType; const PathPoint *points; std::size_t pointCount; };\n+struct AutoStep { AutoStepType type; const char *target; double value; };\n+struct AutoDefinition { const char *name; const AutoStep *steps; std::size_t stepCount; };\n+\n+${pathArrays}\n\n+${autoArrays}\n\n+inline constexpr PathDefinition paths[]{\n+${pathDefinitions}\n+};\n+inline constexpr AutoDefinition autos[]{\n+${autoDefinitions}\n+};\n+}\n+`;
  // Template 以多行字串組合；移除程式產生器內為了閱讀標示的行首 '+'。
  await writeFile(join(projectDirectory, "include", "compiled-autos.h"), header.replace(/^\+/gm, ""), "utf8");
  return { autos: autos.map(auto=>auto.name), paths: paths.map(path=>path.name) };
}

const server = http.createServer(async (request, response) => {
  if (request.url === "/api/shutdown" && request.method === "POST") {
    response.writeHead(200, { "Content-Type": "application/json; charset=utf-8" });
    response.end('{"stopping":true}');
    setTimeout(() => { core.kill();server.close(() => process.exit(0)); }, 50);
    return;
  }
  if (request.url === "/api/events" && request.method === "GET") {
    response.writeHead(200, { "Content-Type": "text/event-stream", "Cache-Control": "no-cache", "Connection": "keep-alive" });
    eventClients.add(response);
    if (latestState) response.write(`data: ${JSON.stringify(latestState)}\n\n`);
    request.on("close", () => eventClients.delete(response));
    return;
  }
  if (request.url === "/api/path" && request.method === "POST") {
    let body = "";
    for await (const chunk of request) { body += chunk;if (body.length > 1024 * 1024) break; }
    try {
      const document = JSON.parse(body);
      const validationError = validatePathDocument(document);
      if (validationError) throw new Error(validationError);
      const safeName = String(document.name || "untitled-path").toLowerCase().replace(/[^a-z0-9_-]+/g, "-").replace(/^-+|-+$/g, "").slice(0, 40) || "untitled-path";
      const pathDirectory = join(projectDirectory, "paths");
      const jsonPath = join(pathDirectory, `${safeName}.json`);
      const headerPath = join(projectDirectory, "include", "generated-path.h");
      await mkdir(pathDirectory, { recursive: true });
      await writeFile(jsonPath, JSON.stringify({ ...document, name: safeName }, null, 2) + "\n", "utf8");
      await writeFile(headerPath, generatePathHeader({ ...document, name: safeName }), "utf8");
      await regenerateCompiledAutosHeader();
      response.writeHead(200, { "Content-Type": "application/json; charset=utf-8" });
      response.end(JSON.stringify({ saved: true, json: `paths/${safeName}.json`, header: "include/generated-path.h" }));
    } catch (error) {
      response.writeHead(400, { "Content-Type": "application/json; charset=utf-8" });
      response.end(JSON.stringify({ error: error.message }));
    }
    return;
  }
  if (request.url === "/api/paths" && request.method === "GET") {
    try {
      const entries = await readdir(join(projectDirectory, "paths"), { withFileTypes: true });
      const names = entries.filter(entry => entry.isFile() && entry.name.endsWith(".json")).map(entry => entry.name.slice(0, -5)).sort();
      response.writeHead(200, { "Content-Type": "application/json; charset=utf-8", "Cache-Control": "no-store" });response.end(JSON.stringify({ paths: names }));
    } catch {
      response.writeHead(200, { "Content-Type": "application/json; charset=utf-8" });response.end('{"paths":[]}');
    }
    return;
  }
  if (request.url === "/api/autos" && request.method === "GET") {
    try {
      const autoDirectory = join(projectDirectory, "autos"),entries = await readdir(autoDirectory, { withFileTypes: true });
      const autos = await Promise.all(entries.filter(entry => entry.isFile() && entry.name.endsWith(".auto.json")).map(async entry => JSON.parse(await readFile(join(autoDirectory, entry.name), "utf8"))));
      response.writeHead(200, { "Content-Type": "application/json; charset=utf-8", "Cache-Control": "no-store" });response.end(JSON.stringify({ autos }));
    } catch { response.writeHead(200, { "Content-Type": "application/json; charset=utf-8" });response.end('{"autos":[]}'); }
    return;
  }
  if (request.url === "/api/generated-auto" && request.method === "GET") {
    try {
      const header = await readFile(join(projectDirectory, "include", "generated-auto.h"), "utf8");
      const name = header.match(/inline constexpr char name\[\] = "([^"]+)"/)?.[1] || "";
      response.writeHead(200, { "Content-Type": "application/json; charset=utf-8", "Cache-Control": "no-store" });response.end(JSON.stringify({ name }));
    } catch { response.writeHead(200, { "Content-Type": "application/json; charset=utf-8" });response.end('{"name":""}'); }
    return;
  }
  if (request.url === "/api/auto-selection" && request.method === "GET") {
    let compiledAutoNames = [];
    try {
      const result = await regenerateCompiledAutosHeader();compiledAutoNames = result.autos;
    } catch {}
    response.writeHead(200, { "Content-Type": "application/json; charset=utf-8", "Cache-Control": "no-store" });
    response.end(JSON.stringify({ selectedAutoName, compiledAutoNames }));
    return;
  }
  if (request.url === "/api/auto-selection" && request.method === "POST") {
    let body = "";for await (const chunk of request) body += chunk;
    try {
      const requestedName = String(JSON.parse(body).name || "none");
      if (requestedName !== "none") {
        const entries = await readdir(join(projectDirectory, "autos"), { withFileTypes: true });
        if (!entries.some(entry => entry.isFile() && entry.name === `${requestedName}.auto.json`)) throw new Error("Auto does not exist.");
      }
      selectedAutoName = requestedName;
      response.writeHead(200, { "Content-Type": "application/json; charset=utf-8" });
      response.end(JSON.stringify({ selected: true, selectedAutoName }));
    } catch (error) {
      response.writeHead(400, { "Content-Type": "application/json; charset=utf-8" });
      response.end(JSON.stringify({ error: error.message }));
    }
    return;
  }
  if (request.url?.startsWith("/api/path-file?") && request.method === "GET") {
    try {
      const name = new URL(request.url, `http://${request.headers.host || "127.0.0.1"}`).searchParams.get("name") || "";
      if (!/^[a-z0-9_-]{1,40}$/i.test(name)) throw new Error("Invalid path name.");
      const document = JSON.parse(await readFile(join(projectDirectory, "paths", `${name}.json`), "utf8"));
      response.writeHead(200, { "Content-Type": "application/json; charset=utf-8", "Cache-Control": "no-store" });response.end(JSON.stringify(document));
    } catch (error) { response.writeHead(404, { "Content-Type": "application/json; charset=utf-8" });response.end(JSON.stringify({ error: error.message })); }
    return;
  }
  if (request.url === "/api/auto-commands" && request.method === "GET") {
    try { const commands = await readAutoCommandDefinitions();response.writeHead(200, { "Content-Type": "application/json; charset=utf-8", "Cache-Control": "no-store" });response.end(JSON.stringify({ commands })); }
    catch (error) { response.writeHead(500, { "Content-Type": "application/json; charset=utf-8" });response.end(JSON.stringify({ error: error.message })); }
    return;
  }
  if (request.url === "/api/auto" && request.method === "POST") {
    let body = "";for await (const chunk of request) { body += chunk;if (body.length > 1024 * 1024) break; }
    try {
      const document = JSON.parse(body),definitions = await readAutoCommandDefinitions(),validationError = validateAutoDocument(document, definitions.map(command => command.id));if (validationError) throw new Error(validationError);
      const safeName = String(document.name || "untitled-auto").toLowerCase().replace(/[^a-z0-9_-]+/g, "-").replace(/^-+|-+$/g, "").slice(0, 40) || "untitled-auto";
      const autoDirectory = join(projectDirectory, "autos"),jsonPath = join(autoDirectory, `${safeName}.auto.json`),headerPath = join(projectDirectory, "include", "generated-auto.h");
      await mkdir(autoDirectory, { recursive: true });await writeFile(jsonPath, JSON.stringify({ ...document, name: safeName }, null, 2) + "\n", "utf8");await writeFile(headerPath, generateAutoHeader({ ...document, name: safeName }), "utf8");await regenerateCompiledAutosHeader();
      response.writeHead(200, { "Content-Type": "application/json; charset=utf-8" });response.end(JSON.stringify({ saved: true, json: `autos/${safeName}.auto.json`, header: "include/generated-auto.h" }));
    } catch (error) { response.writeHead(400, { "Content-Type": "application/json; charset=utf-8" });response.end(JSON.stringify({ error: error.message })); }
    return;
  }
  if (request.url === "/api/state" && request.method === "GET") {
    response.writeHead(200, { "Content-Type": "application/json; charset=utf-8", "Cache-Control": "no-store" });
    response.end(JSON.stringify(latestState || { event: "starting" }));
    return;
  }
  if (request.url === "/api/command" && request.method === "POST") {
    let body = "";
    for await (const chunk of request) body += chunk;
    if (/^(auto |reset)/.test(body.trim())) pendingStates.length = 0;
    if (!/^(mode (tank|mecanum)|drive -?\d+ -?\d+ \d+|mecanum -?\d+ -?\d+ -?\d+ \d+|button \d+ [01]|tap \d+|rpm \d+|auto (closed|coordinate|timed)|auto generated [a-z0-9_-]+|reset|state)$/.test(body.trim())) {
      response.writeHead(400, { "Content-Type": "text/plain; charset=utf-8" });response.end("Invalid simulator command.");return;
    }
    if (body.trim().startsWith("mode ")) {
      currentDriveMode = body.trim().split(" ")[1];
      response.writeHead(202, { "Content-Type": "application/json; charset=utf-8" });response.end(JSON.stringify({ accepted: true, driveMode: currentDriveMode }));return;
    }
    // drive / mecanum 指令本身已明確指定運動模型，不再使用跨分頁的全域模式阻擋。
    // 否則另一個已開啟分頁最後送出的 mode 會讓目前分頁的遙控指令全部失效。
    if (body.trim().startsWith("drive ") || body.trim().startsWith("mecanum ")) {
      const now = Date.now();
      if (now - lastDriveCommandAt < 15) {
        response.writeHead(202, { "Content-Type": "application/json; charset=utf-8" });response.end('{"accepted":false,"reason":"coalesced"}');return;
      }
      lastDriveCommandAt = now;
    }
    if (body.trim().startsWith("auto ")) {
      // 通知所有已開啟分頁這是一輪全新的 Auto，避免上一輪尚未播完的軌跡混入。
      const startPacket = `data: ${JSON.stringify({ event: "auto-start", mode: body.trim().slice(5) })}\n\n`;
      for (const client of eventClients) client.write(startPacket);
    }
    core.stdin.write(body.trim() + "\n");
    response.writeHead(202, { "Content-Type": "application/json; charset=utf-8" });response.end('{"accepted":true}');return;
  }
  const requestPath = new URL(request.url, `http://${request.headers.host || "127.0.0.1"}`).pathname;
  const requestedPath = requestPath === "/" ? "index.html" : requestPath.slice(1);
  const safePath = normalize(requestedPath).replace(/^(\.\.[\\/])+/, "");
  const absolutePath = join(simDirectory, safePath);

  if (!absolutePath.startsWith(simDirectory)) {
    response.writeHead(403, { "Content-Type": "text/plain; charset=utf-8" });
    response.end("Forbidden");
    return;
  }

  try {
    const content = await readFile(absolutePath);
    response.writeHead(200, {
      "Content-Type": mimeTypes[extname(absolutePath)] || "application/octet-stream",
      "Cache-Control": "no-store"
    });
    response.end(content);
  } catch {
    response.writeHead(404, { "Content-Type": "text/plain; charset=utf-8" });
    response.end("VEX simulator file not found.");
  }
});

server.listen(port, "127.0.0.1", () => {
  console.log(`VEX Desktop Simulator: http://127.0.0.1:${port}`);
  console.log("Press Ctrl+C to stop the simulator.");
});

process.on("exit", () => core.kill());
process.on("SIGINT", () => { core.kill();process.exit(0); });
