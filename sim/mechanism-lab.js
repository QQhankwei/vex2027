// Elevator + Arm 姿態模擬器。
// 本檔只負責幾何姿態與教學操作，不會傳送馬達命令到實機。
(() => {
  const tabs = document.querySelector('.tabs');
  const main = document.querySelector('main');
  if (!tabs || !main || document.querySelector('#mechanism')) return;

  // 所有單位集中管理。取得實際機器尺寸後，只需要調整這個物件。
  const mechanismParameters = Object.freeze({
    elevatorMinHeightIn: 0.0,
    elevatorMaxHeightIn: 32.0,
    armMinAngleDeg: -20.0,
    armMaxAngleDeg: 135.0,
    armLengthIn: 18.0,
    carriageBaseHeightIn: 8.0
  });

  const presets = Object.freeze({
    stowed: { label: 'STOWED／收納', heightIn: 0, angleDeg: 90 },
    intake: { label: 'INTAKE／取物', heightIn: 2, angleDeg: 5 },
    low: { label: 'LOW SCORE／低位', heightIn: 12, angleDeg: 35 },
    high: { label: 'HIGH SCORE／高位', heightIn: 30, angleDeg: 65 }
  });

  const tab = document.createElement('button');
  tab.dataset.page = 'mechanism';
  tab.textContent = 'Mechanism Lab／機構姿態';
  tabs.append(tab);

  const page = document.createElement('section');
  page.id = 'mechanism';
  page.className = 'page mechanism-page';
  page.innerHTML = `
    <section class="mechanism-stage">
      <header class="mechanism-stage-header"><b>Elevator + Arm Pose／升降手臂姿態</b><span>SIDE PROFILE／側視圖</span></header>
      <svg class="mechanism-canvas" viewBox="0 0 720 570" role="img" aria-label="Elevator and arm posture simulator">
        <g class="mechanism-grid">
          ${[90,180,270,360,450,540,630].map(x => `<line x1="${x}" y1="25" x2="${x}" y2="525"/>`).join('')}
          ${[75,150,225,300,375,450,525].map(y => `<line x1="35" y1="${y}" x2="685" y2="${y}"/>`).join('')}
        </g>
        <line class="mechanism-floor" x1="35" y1="500" x2="685" y2="500"/>
        <g id="mechanismRobot">
          <rect class="mechanism-chassis" x="220" y="430" width="280" height="58" rx="10"/>
          <circle class="mechanism-wheel" cx="270" cy="490" r="27"/><circle class="mechanism-wheel" cx="450" cy="490" r="27"/>
          <g id="elevatorStages">
            <line class="mechanism-rail stage-1" x1="315" y1="426" x2="315" y2="300"/><line class="mechanism-rail stage-1" x1="405" y1="426" x2="405" y2="300"/>
            <g id="elevatorStage2"><line class="mechanism-rail stage-2" x1="323" y1="426" x2="323" y2="300"/><line class="mechanism-rail stage-2" x1="397" y1="426" x2="397" y2="300"/></g>
            <g id="elevatorStage3"><line class="mechanism-rail stage-3" x1="331" y1="426" x2="331" y2="300"/><line class="mechanism-rail stage-3" x1="389" y1="426" x2="389" y2="300"/></g>
            <g id="elevatorStage4"><line class="mechanism-rail stage-4" x1="339" y1="426" x2="339" y2="300"/><line class="mechanism-rail stage-4" x1="381" y1="426" x2="381" y2="300"/></g>
          </g>
          <g id="mechanismCarriage">
            <rect class="mechanism-carriage" x="300" y="390" width="120" height="34" rx="5"/>
            <g id="mechanismArm">
              <line class="mechanism-arm-outline" x1="360" y1="407" x2="520" y2="407"/>
              <line class="mechanism-arm" x1="360" y1="407" x2="520" y2="407"/>
              <circle class="mechanism-pivot" cx="360" cy="407" r="12"/>
              <path class="mechanism-tool" d="M508 388 L548 395 L548 419 L508 426 Z"/>
            </g>
          </g>
          <line id="mechanismHeightLine" class="mechanism-dimension" x1="190" y1="407" x2="285" y2="407"/>
          <text id="mechanismHeightText" class="mechanism-dimension-text" x="190" y="398">0.0 in</text>
          <text id="mechanismGroundWarning" class="mechanism-ground-warning" x="455" y="470" hidden>GROUND COLLISION／碰地</text>
        </g>
      </svg>
    </section>
    <aside class="mechanism-controls">
      <section class="mechanism-panel">
        <header class="mechanism-panel-title"><b>Pose Command／姿態命令</b><span>SIMULATION ONLY</span></header>
        <div class="mechanism-panel-body">
          <div class="mechanism-control"><label for="elevatorHeight"><span>Elevator height／升降高度</span><b id="elevatorHeightValue">0.0 in</b></label><input id="elevatorHeight" type="range" min="0" max="32" step="0.1" value="0"></div>
          <div class="mechanism-control"><label for="armAngle"><span>Arm angle／手臂角度</span><b id="armAngleValue">90.0°</b></label><input id="armAngle" type="range" min="-20" max="135" step="0.5" value="90"></div>
          <div id="mechanismStatus" class="mechanism-status">POSE VALID／姿態有效</div>
        </div>
      </section>
      <section class="mechanism-panel">
        <header class="mechanism-panel-title"><b>Preset Poses／預設姿態</b><span>TEACHING EXAMPLES</span></header>
        <div class="mechanism-panel-body"><div id="mechanismPresets" class="mechanism-preset-grid"></div></div>
      </section>
      <section class="mechanism-panel">
        <header class="mechanism-panel-title"><b>Continuous Motion／連續動作</b><span>4-STAGE ELEVATOR DEMO</span></header>
        <div class="mechanism-panel-body">
          <div id="mechanismTimeline" class="mechanism-timeline">${Array.from({length:5},(_,index)=>`<i class="mechanism-timeline-step" data-motion-step="${index}"></i>`).join('')}</div>
          <div class="mechanism-speed"><span>Transition speed／動作速度</span><select id="mechanismMotionSpeed"><option value="1800">SLOW／慢速</option><option value="1100" selected>NORMAL／正常</option><option value="650">FAST／快速</option></select></div>
          <div class="mechanism-sequence-actions"><button id="mechanismPlaySequence" class="control">▶ PLAY／播放</button><button id="mechanismStopSequence" class="control">■ STOP／停止</button></div>
        </div>
      </section>
      <section class="mechanism-panel">
        <header class="mechanism-panel-title"><b>Calculated Pose／計算結果</b><span>INCHES + DEGREES</span></header>
        <div class="mechanism-panel-body">
          <div class="mechanism-readout">
            <div class="mechanism-metric"><small>Tool X／末端 X</small><b id="toolXValue">0.0 in</b></div>
            <div class="mechanism-metric"><small>Tool Y／末端 Y</small><b id="toolYValue">0.0 in</b></div>
          </div>
          <pre id="mechanismCode" class="mechanism-code">setElevatorArmPose(0.0, 90.0);</pre>
        </div>
      </section>
    </aside>`;
  main.append(page);

  const $ = selector => document.querySelector(selector);
  const clamp = (value, min, max) => Math.max(min, Math.min(max, Number(value)));
  const state = { heightIn: 0, angleDeg: 90 };
  let motionToken = 0;
  let sequenceToken = 0;

  // Carriage 與 Arm 必須實際掛在最內側第 4 階，而不是只使用相同位移數值。
  // 因為 SVG transform 會由父層傳遞，第 4 階移動時整組末端機構必定一起移動。
  $('#elevatorStage4').append($('#mechanismCarriage'));

  function renderPose() {
    const p = mechanismParameters;
    const heightRatio = (state.heightIn - p.elevatorMinHeightIn) / (p.elevatorMaxHeightIn - p.elevatorMinHeightIn);
    // 四階 Elevator：每一活動階分攤總行程，呈現連續伸縮關係。
    const stageTravel = heightRatio * 95;
    const carriageY = 390 - stageTravel * 3;
    const svgArmAngle = -state.angleDeg;
    const radians = state.angleDeg * Math.PI / 180;
    const toolXIn = Math.cos(radians) * p.armLengthIn;
    const toolYIn = p.carriageBaseHeightIn + state.heightIn + Math.sin(radians) * p.armLengthIn;
    const groundCollision = toolYIn < 0;

    $('#elevatorStage2').setAttribute('transform', `translate(0 ${-stageTravel})`);
    $('#elevatorStage3').setAttribute('transform', `translate(0 ${-stageTravel * 2})`);
    $('#elevatorStage4').setAttribute('transform', `translate(0 ${-stageTravel * 3})`);
    $('#mechanismArm').setAttribute('transform', `rotate(${svgArmAngle} 360 407)`);
    $('#mechanismHeightLine').setAttribute('y1', carriageY + 17);
    $('#mechanismHeightLine').setAttribute('y2', carriageY + 17);
    $('#mechanismHeightText').setAttribute('y', carriageY + 8);
    $('#mechanismHeightText').textContent = `${state.heightIn.toFixed(1)} in`;
    $('#mechanismGroundWarning').toggleAttribute('hidden', !groundCollision);
    $('#elevatorHeight').value = state.heightIn;
    $('#armAngle').value = state.angleDeg;
    $('#elevatorHeightValue').textContent = `${state.heightIn.toFixed(1)} in`;
    $('#armAngleValue').textContent = `${state.angleDeg.toFixed(1)}°`;
    $('#toolXValue').textContent = `${toolXIn.toFixed(1)} in`;
    $('#toolYValue').textContent = `${toolYIn.toFixed(1)} in`;
    $('#mechanismCode').textContent = `setElevatorArmPose(${state.heightIn.toFixed(1)}, ${state.angleDeg.toFixed(1)});`;
    $('#mechanismStatus').classList.toggle('warn', groundCollision);
    $('#mechanismStatus').textContent = groundCollision ? 'GROUND COLLISION／手臂末端碰地' : 'POSE VALID／姿態有效';
    document.querySelectorAll('.mechanism-preset').forEach(button => {
      const preset = presets[button.dataset.preset];
      button.classList.toggle('active', preset.heightIn === state.heightIn && preset.angleDeg === state.angleDeg);
    });
  }

  /**
   * 設定 Elevator + Arm 的模擬姿態。
   * @param {number} elevatorHeightIn 升降高度，單位 inch。
   * @param {number} armAngleDeg Arm 相對水平面的角度，向上為正，單位 degree。
   */
  window.setElevatorArmPose = (elevatorHeightIn, armAngleDeg) => {
    sequenceToken++;
    motionToken++;
    const p = mechanismParameters;
    state.heightIn = clamp(elevatorHeightIn, p.elevatorMinHeightIn, p.elevatorMaxHeightIn);
    state.angleDeg = clamp(armAngleDeg, p.armMinAngleDeg, p.armMaxAngleDeg);
    renderPose();
    return { ...state };
  };

  /**
   * 以連續動畫移動到指定姿態。
   * Elevator 與 Arm 同時依平滑曲線運動，方便觀察中途是否碰撞。
   * @returns {Promise<boolean>} true 代表完成；false 代表被新命令中止。
   */
  window.moveElevatorArmPose = (elevatorHeightIn, armAngleDeg, durationMs = 1100) => {
    const p = mechanismParameters;
    const targetHeight = clamp(elevatorHeightIn, p.elevatorMinHeightIn, p.elevatorMaxHeightIn);
    const targetAngle = clamp(armAngleDeg, p.armMinAngleDeg, p.armMaxAngleDeg);
    const startHeight = state.heightIn;
    const startAngle = state.angleDeg;
    const duration = Math.max(100, Number(durationMs));
    const token = ++motionToken;
    const startTime = performance.now();
    $('#mechanismStatus').textContent = 'MOVING／機構移動中';

    return new Promise(resolve => {
      function animate(now) {
        if (token !== motionToken) { resolve(false); return; }
        const progress = Math.min(1, (now - startTime) / duration);
        // Smoothstep：起步與停止都減速，較接近真實機構而非線性瞬移。
        const eased = progress * progress * (3 - 2 * progress);
        state.heightIn = startHeight + (targetHeight - startHeight) * eased;
        state.angleDeg = startAngle + (targetAngle - startAngle) * eased;
        renderPose();
        if (progress < 1) requestAnimationFrame(animate);
        else resolve(true);
      }
      requestAnimationFrame(animate);
    });
  };

  $('#elevatorHeight').addEventListener('input', event => window.setElevatorArmPose(event.target.value, state.angleDeg));
  $('#armAngle').addEventListener('input', event => window.setElevatorArmPose(state.heightIn, event.target.value));
  $('#mechanismPresets').innerHTML = Object.entries(presets).map(([id, pose]) => `<button class="mechanism-preset" data-preset="${id}">${pose.label}<small>${pose.heightIn.toFixed(1)} in · ${pose.angleDeg.toFixed(1)}°</small></button>`).join('');
  document.querySelectorAll('.mechanism-preset').forEach(button => button.addEventListener('click', () => {
    const pose = presets[button.dataset.preset];
    sequenceToken++;
    window.moveElevatorArmPose(pose.heightIn, pose.angleDeg, Number($('#mechanismMotionSpeed').value));
  }));

  const continuousSequence = [presets.stowed, presets.intake, { label: 'CLEARANCE／抬升避障', heightIn: 14, angleDeg: 45 }, presets.high, presets.stowed];
  function setTimeline(activeIndex, completeThrough = -1) {
    document.querySelectorAll('.mechanism-timeline-step').forEach((step, index) => {
      step.classList.toggle('active', index === activeIndex);
      step.classList.toggle('complete', index <= completeThrough);
    });
  }
  $('#mechanismPlaySequence').onclick = async () => {
    const currentSequenceToken = ++sequenceToken;
    const duration = Number($('#mechanismMotionSpeed').value);
    for (let index = 0; index < continuousSequence.length; index++) {
      if (currentSequenceToken !== sequenceToken) return;
      setTimeline(index, index - 1);
      const pose = continuousSequence[index];
      const completed = await window.moveElevatorArmPose(pose.heightIn, pose.angleDeg, duration);
      if (!completed || currentSequenceToken !== sequenceToken) return;
      if (index < continuousSequence.length - 1) await new Promise(resolve => setTimeout(resolve, 180));
    }
    setTimeline(-1, continuousSequence.length - 1);
    $('#mechanismStatus').textContent = 'SEQUENCE COMPLETE／連續動作完成';
  };
  $('#mechanismStopSequence').onclick = () => {
    sequenceToken++;
    motionToken++;
    setTimeline(-1, -1);
    $('#mechanismStatus').textContent = 'MOTION STOPPED／動作已停止';
  };

  // dashboard-v2 已處理既有頁籤；動態加入的機構頁籤在此補上相同行為。
  tab.addEventListener('click', () => {
    document.querySelectorAll('.page').forEach(node => node.classList.toggle('active', node.id === 'mechanism'));
    document.querySelectorAll('.tabs button').forEach(node => node.classList.toggle('active', node === tab));
  });
  window.mechanismParameters = mechanismParameters;
  window.setElevatorArmPose(0, 90);
})();
