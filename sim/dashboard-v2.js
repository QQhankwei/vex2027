// RIFTIORY Operations Dashboard 2.0
// 僅讀取既有 telemetry 與 Auto selection，不直接控制底盤或機構。
(() => {
  const tabs = document.querySelector('.tabs');
  const main = document.querySelector('main');
  if (!tabs || !main || document.querySelector('#overview')) return;

  const overviewTab = document.createElement('button');
  overviewTab.dataset.page = 'overview';
  overviewTab.textContent = 'Operations／作戰總覽';
  tabs.prepend(overviewTab);

  const page = document.createElement('section');
  page.id = 'overview';
  page.className = 'page operations-page';
  page.innerHTML = `
    <section class="ops-hero">
      <div>
        <div class="ops-kicker">RIFTIORY // MATCH OPERATIONS</div>
        <div class="ops-title">Robot ready at a glance.</div>
        <div class="ops-subtitle">機器狀態、比賽時間與自動程式集中監看</div>
        <div class="ops-live-line"><i class="ops-live-dot"></i><span id="opsConnection">SIM CORE CONNECTED／模擬核心已連線</span></div>
      </div>
      <div class="ops-clock-wrap">
        <div><div class="ops-kicker">MATCH CLOCK／比賽時間</div><div id="opsClock" class="ops-clock">2:00.0</div><div id="opsPhase" class="ops-phase">READY／準備</div></div>
        <div class="ops-clock-actions"><button id="opsStart" class="control">START／開始</button><button id="opsReset" class="control">RESET／重設</button></div>
      </div>
      <div class="ops-hero-auto">
        <div class="ops-kicker">SELECTED AUTO／自動程式</div>
        <div id="opsAutoName" class="ops-auto-name">NO AUTO</div>
        <div id="opsAutoState" class="ops-auto-state">SAFE DISABLED／安全停用</div>
      </div>
    </section>
    <section class="ops-grid">
      <article class="ops-card">
        <header class="ops-card-header"><b>System Health／系統健康</b><span>LIVE TELEMETRY</span></header>
        <div class="ops-card-body ops-health-grid">
          <div id="opsBatteryCard" class="ops-health"><small>Battery／電量</small><strong id="opsBattery">100%</strong></div>
          <div id="opsThermalCard" class="ops-health"><small>Motor Max／最高溫</small><strong id="opsTemperature">25.0°C</strong></div>
          <div class="ops-health"><small>Left Drive／左側底盤</small><strong id="opsLeftRpm">0 RPM</strong></div>
          <div class="ops-health"><small>Right Drive／右側底盤</small><strong id="opsRightRpm">0 RPM</strong></div>
        </div>
      </article>
      <article class="ops-card">
        <header class="ops-card-header"><b>Robot Pose／機器人姿態</b><span>FIELD FRAME</span></header>
        <div class="ops-card-body ops-pose-layout">
          <div class="ops-compass"><div id="opsHeadingArrow" class="ops-heading-arrow"></div></div>
          <div class="ops-pose-values">
            <div class="ops-pose-value"><span>X Position</span><b id="opsX">0.0 in</b></div>
            <div class="ops-pose-value"><span>Y Position</span><b id="opsY">0.0 in</b></div>
            <div class="ops-pose-value"><span>Heading</span><b id="opsHeading">0.0°</b></div>
          </div>
        </div>
      </article>
      <article class="ops-card">
        <header class="ops-card-header"><b>Drive Output／底盤輸出</b><span>±12 VOLTS</span></header>
        <div class="ops-card-body">
          <div class="ops-drive-row"><div class="ops-drive-label"><span>LEFT／左側</span><b id="opsLeftVolt">0.0 V</b></div><div class="ops-drive-track"><div id="opsLeftBar" class="ops-drive-fill"></div></div></div>
          <div class="ops-drive-row"><div class="ops-drive-label"><span>RIGHT／右側</span><b id="opsRightVolt">0.0 V</b></div><div class="ops-drive-track"><div id="opsRightBar" class="ops-drive-fill"></div></div></div>
          <div class="ops-footer-note">50% = 0 V · BAR DIRECTION REPRESENTS SIGN／中央為零，方向代表正負</div>
        </div>
      </article>
      <article class="ops-card">
        <header class="ops-card-header"><b>Match Readiness／上場檢查</b><span id="opsReadyCount">3 / 4 READY</span></header>
        <div class="ops-card-body ops-readiness">
          <div class="ops-ready-row"><i class="ops-ready-dot"></i><span>Telemetry Link／遙測連線</span><b id="opsReadyLink">CONNECTED</b></div>
          <div class="ops-ready-row"><i class="ops-ready-dot"></i><span>IMU／陀螺儀</span><b id="opsReadyImu">READY</b></div>
          <div id="opsReadyBatteryRow" class="ops-ready-row"><i class="ops-ready-dot"></i><span>Battery／電池</span><b id="opsReadyBattery">GOOD</b></div>
          <div id="opsReadyAutoRow" class="ops-ready-row warn"><i class="ops-ready-dot"></i><span>Autonomous／自動程式</span><b id="opsReadyAuto">SELECT</b></div>
        </div>
      </article>
    </section>`;
  main.prepend(page);

  const select = selector => document.querySelector(selector);
  const setText = (selector, text) => { const node = select(selector); if (node) node.textContent = text; };
  const showPage = pageName => {
    document.querySelectorAll('.page').forEach(node => node.classList.toggle('active', node.id === pageName));
    document.querySelectorAll('.tabs button').forEach(node => node.classList.toggle('active', node.dataset.page === pageName));
  };
  document.querySelectorAll('.tabs button').forEach(button => button.addEventListener('click', () => showPage(button.dataset.page)));
  showPage('overview');

  // 使用既有計時器按鈕作為唯一狀態來源，避免兩套計時邏輯產生不同結果。
  select('#opsStart').onclick = () => select('#matchStart')?.click();
  select('#opsReset').onclick = () => select('#matchReset')?.click();
  const syncClock = () => {
    setText('#opsClock', select('#matchClock')?.textContent || '2:00.0');
    setText('#opsPhase', select('#matchPhase')?.textContent || 'READY／準備');
    setText('#opsStart', select('#matchStart')?.textContent || 'START／開始');
    requestAnimationFrame(syncClock);
  };
  requestAnimationFrame(syncClock);

  const setReady = (rowSelector, valueSelector, ready, goodText, warnText) => {
    select(rowSelector)?.classList.toggle('warn', !ready);
    setText(valueSelector, ready ? goodText : warnText);
  };
  const updateReadyCount = () => {
    const ready = document.querySelectorAll('.ops-ready-row:not(.warn)').length;
    setText('#opsReadyCount', `${ready} / 4 READY`);
  };

  window.addEventListener('vex-telemetry', event => {
    const packet = event.detail;
    const battery = Number(packet.batteryPct ?? 100);
    const temperature = Number(packet.hottestDriveC ?? 25);
    const heading = Number(packet.headingDeg ?? packet.imuHeadingDeg ?? 0);
    const leftVolt = Number(packet.leftVolt ?? 0);
    const rightVolt = Number(packet.rightVolt ?? 0);
    setText('#opsBattery', `${Math.round(battery)}%`);
    setText('#opsTemperature', `${temperature.toFixed(1)}°C`);
    setText('#opsLeftRpm', `${Math.round(packet.leftRpm ?? 0)} RPM`);
    setText('#opsRightRpm', `${Math.round(packet.rightRpm ?? 0)} RPM`);
    setText('#opsX', `${Number(packet.xIn ?? 0).toFixed(1)} in`);
    setText('#opsY', `${Number(packet.yIn ?? 0).toFixed(1)} in`);
    setText('#opsHeading', `${heading.toFixed(1)}°`);
    select('#opsHeadingArrow')?.style.setProperty('--heading', `${heading}deg`);
    setText('#opsLeftVolt', `${leftVolt.toFixed(1)} V`);
    setText('#opsRightVolt', `${rightVolt.toFixed(1)} V`);
    select('#opsLeftBar')?.style.setProperty('width', `${Math.max(0, Math.min(100, 50 + leftVolt / 12 * 50))}%`);
    select('#opsRightBar')?.style.setProperty('width', `${Math.max(0, Math.min(100, 50 + rightVolt / 12 * 50))}%`);
    select('#opsBatteryCard')?.style.setProperty('--level', `${Math.max(0, Math.min(100, battery))}%`);
    select('#opsThermalCard')?.style.setProperty('--level', `${Math.max(0, Math.min(100, temperature / 70 * 100))}%`);
    select('#opsThermalCard')?.style.setProperty('--tone', temperature < 50 ? 'var(--ops-success)' : 'var(--ops-danger)');
    setReady('#opsReadyBatteryRow', '#opsReadyBattery', battery >= 30, 'GOOD', 'LOW');
    updateReadyCount();
  });

  async function updateAutoSelection() {
    try {
      const data = await fetch('/api/auto-selection').then(response => response.json());
      const name = data.selectedAutoName || 'none';
      const ready = name !== 'none' && (data.compiledAutoNames || []).includes(name);
      setText('#opsAutoName', name === 'none' ? 'NO AUTO' : name);
      setText('#opsAutoState', ready ? 'READY TO RUN／可以執行' : name === 'none' ? 'SAFE DISABLED／安全停用' : 'REBUILD REQUIRED／需要重新編譯');
      setReady('#opsReadyAutoRow', '#opsReadyAuto', ready, 'READY', name === 'none' ? 'SELECT' : 'REBUILD');
      updateReadyCount();
    } catch {
      setText('#opsAutoState', 'DATA UNAVAILABLE／無法讀取');
    }
  }
  updateAutoSelection();
  window.addEventListener('focus', updateAutoSelection);
})();
