const char INDEX_HTML[] = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Advanced Car Distance Dashboard</title>
  <style>
    :root {
      --bg: #08111e;
      --panel: rgba(10, 18, 33, 0.82);
      --panel-2: rgba(15, 26, 46, 0.94);
      --line: rgba(148, 163, 184, 0.18);
      --text: #ecf5ff;
      --muted: #93a4c7;
      --accent: #38bdf8;
      --accent-2: #4ade80;
      --danger: #fb7185;
      --shadow: 0 28px 80px rgba(0, 0, 0, 0.35);
    }

    * { box-sizing: border-box; }

    body {
      margin: 0;
      color: var(--text);
      font-family: Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      background:
        radial-gradient(circle at top left, rgba(56, 189, 248, 0.22), transparent 32%),
        radial-gradient(circle at top right, rgba(74, 222, 128, 0.14), transparent 28%),
        linear-gradient(180deg, #07111f 0%, #091425 48%, #050b14 100%);
      min-height: 100vh;
    }

    .wrap {
      max-width: 1360px;
      margin: 0 auto;
      padding: 22px;
    }

    .hero {
      display: grid;
      grid-template-columns: 1.4fr 0.95fr;
      gap: 18px;
      margin-bottom: 18px;
    }

    .panel {
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 22px;
      box-shadow: var(--shadow);
      backdrop-filter: blur(14px);
    }

    .hero-main {
      padding: 26px;
      position: relative;
      overflow: hidden;
    }

    .hero-main::after {
      content: "";
      position: absolute;
      right: -36px;
      bottom: -54px;
      width: 220px;
      height: 220px;
      border-radius: 50%;
      background: radial-gradient(circle, rgba(74, 222, 128, 0.20), transparent 70%);
      pointer-events: none;
      filter: blur(8px);
    }

    .eyebrow {
      text-transform: uppercase;
      letter-spacing: 0.22em;
      font-size: 11px;
      color: var(--accent);
      margin-bottom: 10px;
    }

    h1, h2, h3, p { margin: 0; }

    h1 {
      font-size: clamp(28px, 4vw, 52px);
      line-height: 1.02;
    }

    .subtitle {
      margin-top: 14px;
      color: var(--muted);
      max-width: 70ch;
      line-height: 1.65;
      font-size: 15px;
    }

    .toolbar {
      display: flex;
      flex-wrap: wrap;
      gap: 12px;
      margin-top: 20px;
    }

    button {
      border: 0;
      border-radius: 16px;
      padding: 13px 18px;
      cursor: pointer;
      font-weight: 700;
      font-size: 14px;
      transition: transform 0.15s ease, box-shadow 0.15s ease, opacity 0.15s ease;
      box-shadow: 0 12px 28px rgba(0, 0, 0, 0.22);
    }

    button:hover { transform: translateY(-1px); }

    .btn-start { background: linear-gradient(135deg, #4ade80, #86efac); color: #06101c; }
    .btn-stop { background: linear-gradient(135deg, #fb7185, #fda4af); color: #06101c; }
    .btn-reset { background: linear-gradient(135deg, #38bdf8, #7dd3fc); color: #06101c; }
    .btn-save { background: linear-gradient(135deg, #fbbf24, #fde68a); color: #06101c; }
    .btn-secondary { background: rgba(255,255,255,0.05); color: var(--text); border: 1px solid var(--line); box-shadow: none; }

    .stats-grid {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 12px;
    }

    .mini-card {
      background: var(--panel-2);
      border: 1px solid var(--line);
      border-radius: 18px;
      padding: 16px;
      min-height: 88px;
    }

    .mini-card .label {
      color: var(--muted);
      font-size: 12px;
      margin-bottom: 8px;
    }

    .mini-card .value {
      font-size: 22px;
      font-weight: 800;
      line-height: 1.1;
    }

    .status-row {
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      margin-top: 16px;
    }

    .pill {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      padding: 10px 12px;
      border-radius: 999px;
      border: 1px solid var(--line);
      background: rgba(255,255,255,0.04);
      color: var(--text);
      font-size: 12px;
    }

    .pill .dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: var(--accent-2);
      box-shadow: 0 0 0 4px rgba(74, 222, 128, 0.12);
    }

    .grid-2 {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 18px;
      margin-bottom: 18px;
    }

    .card {
      padding: 20px;
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 22px;
      box-shadow: var(--shadow);
      backdrop-filter: blur(14px);
    }

    .card h2 {
      font-size: 18px;
      margin-bottom: 14px;
    }

    .subhead {
      color: var(--muted);
      font-size: 12px;
      margin-bottom: 14px;
      text-transform: uppercase;
      letter-spacing: 0.16em;
    }

    .metric-list {
      display: grid;
      grid-template-columns: repeat(3, minmax(0, 1fr));
      gap: 12px;
    }

    .metric {
      background: rgba(255,255,255,0.03);
      border: 1px solid var(--line);
      border-radius: 16px;
      padding: 14px;
      min-height: 74px;
    }

    .metric .k {
      font-size: 12px;
      color: var(--muted);
      margin-bottom: 8px;
    }

    .metric .v {
      font-size: 20px;
      font-weight: 800;
    }

    .metric .u {
      font-size: 12px;
      color: var(--muted);
      margin-left: 4px;
      font-weight: 600;
    }

    .graph-toolbar {
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      margin-bottom: 12px;
    }

    .series-grid {
      display: grid;
      grid-template-columns: repeat(4, minmax(0, 1fr));
      gap: 10px;
      margin-bottom: 14px;
    }

    .check {
      display: flex;
      align-items: center;
      gap: 10px;
      padding: 10px 12px;
      border-radius: 14px;
      border: 1px solid var(--line);
      background: rgba(255,255,255,0.03);
      font-size: 13px;
    }

    .check input { accent-color: var(--accent); }

    .canvas-shell {
      position: relative;
      height: 360px;
      border-radius: 18px;
      overflow: hidden;
      border: 1px solid var(--line);
      background: linear-gradient(180deg, rgba(8, 15, 28, 0.95), rgba(6, 12, 22, 0.95));
    }

    canvas {
      width: 100%;
      height: 100%;
      display: block;
    }

    .legend {
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      margin-top: 12px;
      color: var(--muted);
      font-size: 12px;
    }

    .legend span {
      display: inline-flex;
      align-items: center;
      gap: 6px;
    }

    .legend i {
      width: 10px;
      height: 10px;
      border-radius: 50%;
      display: inline-block;
    }

    .settings-grid {
      display: grid;
      grid-template-columns: repeat(4, minmax(0, 1fr));
      gap: 12px;
    }

    .field {
      background: rgba(255,255,255,0.03);
      border: 1px solid var(--line);
      border-radius: 16px;
      padding: 12px;
    }

    .field label {
      display: block;
      font-size: 12px;
      color: var(--muted);
      margin-bottom: 8px;
    }

    .field input {
      width: 100%;
      border: 1px solid rgba(255,255,255,0.08);
      background: rgba(3, 7, 18, 0.54);
      color: var(--text);
      border-radius: 12px;
      padding: 12px 12px;
      font-size: 14px;
      outline: none;
    }

    .note {
      color: var(--muted);
      font-size: 12px;
      line-height: 1.6;
      margin-top: 12px;
    }

    .log {
      margin-top: 12px;
      min-height: 180px;
      max-height: 240px;
      overflow: auto;
      padding: 14px;
      border-radius: 16px;
      border: 1px solid var(--line);
      background: rgba(3, 7, 18, 0.68);
      font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
      font-size: 12px;
      line-height: 1.5;
      color: #c7d2fe;
      white-space: pre-wrap;
    }

    .footer {
      margin-top: 10px;
      color: var(--muted);
      font-size: 12px;
    }

    @media (max-width: 1100px) {
      .hero, .grid-2, .metric-list, .settings-grid, .series-grid {
        grid-template-columns: repeat(2, minmax(0, 1fr));
      }
    }

    @media (max-width: 760px) {
      .hero, .grid-2 {
        grid-template-columns: 1fr;
      }
      .metric-list, .settings-grid, .series-grid {
        grid-template-columns: 1fr;
      }
      .canvas-shell { height: 300px; }
    }
  </style>
</head>
<body>
  <div class="wrap">
    <section class="hero">
      <div class="panel hero-main">
        <div class="eyebrow">Advanced Car Distance Monitor</div>
        <h1>Raw vs Kalman Motion Dashboard</h1>
        <p class="subtitle">Track encoder-based travel, raw MPU motion, and Kalman-filtered motion side by side. The robot will drive forward until the encoder distance reaches 1 meter, while the dashboard streams both noisy and filtered attitude and distance estimates.</p>

        <div class="toolbar">
          <button class="btn-start" onclick="sendAction('/api/start')">Start Run</button>
          <button class="btn-stop" onclick="sendAction('/api/stop')">Stop</button>
          <button class="btn-reset" onclick="sendAction('/api/reset')">Reset</button>
          <button class="btn-secondary" onclick="reloadKalman()">Reload Kalman</button>
          <button class="btn-secondary" onclick="startCalibration(30)">Calibrate MPU (30s)</button>
        </div>

        <div class="status-row">
          <div class="pill"><span class="dot"></span><span id="runState">Idle</span></div>
          <div class="pill">WiFi: <span id="wifiMode">---</span></div>
          <div class="pill">SSID: <span id="ssid">---</span></div>
          <div class="pill">IP: <span id="ip">---</span></div>
        </div>
      </div>

      <div class="panel" style="padding: 18px;">
        <div class="stats-grid">
          <div class="mini-card">
            <div class="label">Encoder Distance</div>
            <div class="value"><span id="encDist">0.000</span> <span class="u">m</span></div>
          </div>
          <div class="mini-card">
            <div class="label">Target</div>
            <div class="value"><span id="targetDist">1.000</span> <span class="u">m</span></div>
          </div>
          <div class="mini-card">
            <div class="label">Left Ticks</div>
            <div class="value" id="leftTicks">0</div>
          </div>
          <div class="mini-card">
            <div class="label">Right Ticks</div>
            <div class="value" id="rightTicks">0</div>
          </div>
        </div>
      </div>
    </section>

    <section class="grid-2">
      <div class="card">
        <div class="subhead">Without Kalman</div>
        <h2>Raw MPU</h2>
        <div class="metric-list">
          <div class="metric"><div class="k">Roll</div><div class="v"><span id="rawRoll">0.00</span><span class="u">deg</span></div></div>
          <div class="metric"><div class="k">Pitch</div><div class="v"><span id="rawPitch">0.00</span><span class="u">deg</span></div></div>
          <div class="metric"><div class="k">Yaw</div><div class="v"><span id="rawYaw">0.00</span><span class="u">deg</span></div></div>
          <div class="metric"><div class="k">Distance</div><div class="v"><span id="rawDistance">0.000</span><span class="u">m</span></div></div>
          <div class="metric"><div class="k">Velocity</div><div class="v"><span id="rawVelocity">0.000</span><span class="u">m/s</span></div></div>
          <div class="metric"><div class="k">Accel</div><div class="v"><span id="rawAccel">0.000</span><span class="u">m/s²</span></div></div>
        </div>
      </div>

      <div class="card">
        <div class="subhead">With Kalman</div>
        <h2>Filtered MPU</h2>
        <div class="metric-list">
          <div class="metric"><div class="k">Roll</div><div class="v"><span id="kalmanRoll">0.00</span><span class="u">deg</span></div></div>
          <div class="metric"><div class="k">Pitch</div><div class="v"><span id="kalmanPitch">0.00</span><span class="u">deg</span></div></div>
          <div class="metric"><div class="k">Yaw</div><div class="v"><span id="kalmanYaw">0.00</span><span class="u">deg</span></div></div>
          <div class="metric"><div class="k">Distance</div><div class="v"><span id="kalmanDistance">0.000</span><span class="u">m</span></div></div>
          <div class="metric"><div class="k">Velocity</div><div class="v"><span id="kalmanVelocity">0.000</span><span class="u">m/s</span></div></div>
          <div class="metric"><div class="k">Accel</div><div class="v"><span id="kalmanAccel">0.000</span><span class="u">m/s²</span></div></div>
        </div>
      </div>
    </section>

    <section class="card">
      <div class="subhead">Live Plot</div>
      <h2>Selectable Graph</h2>
      <div class="graph-toolbar">
        <div class="pill">Choose the series you want to see</div>
      </div>
      <div class="series-grid">
        <label class="check"><input type="checkbox" id="series-rawRoll" checked /> Raw Roll</label>
        <label class="check"><input type="checkbox" id="series-rawPitch" checked /> Raw Pitch</label>
        <label class="check"><input type="checkbox" id="series-rawYaw" /> Raw Yaw</label>
        <label class="check"><input type="checkbox" id="series-rawDistance" /> Raw Distance</label>
        <label class="check"><input type="checkbox" id="series-kalmanRoll" checked /> Kalman Roll</label>
        <label class="check"><input type="checkbox" id="series-kalmanPitch" checked /> Kalman Pitch</label>
        <label class="check"><input type="checkbox" id="series-kalmanYaw" /> Kalman Yaw</label>
        <label class="check"><input type="checkbox" id="series-kalmanDistance" /> Kalman Distance</label>
      </div>
      <div class="canvas-shell"><canvas id="plot"></canvas></div>
      <div class="legend" id="legend"></div>
      <div class="footer">The graph rescales automatically to the checked series. Select only the lines you want to compare.</div>
    </section>

    <section class="grid-2">
      <div class="card">
        <div class="subhead">EEPROM Settings</div>
        <h2>Kalman Tuning</h2>
        <div class="settings-grid" id="kalmanForm">
          <div class="field"><label>Q Angle</label><input id="qAngle" type="number" step="0.0001" /></div>
          <div class="field"><label>Q Bias</label><input id="qBias" type="number" step="0.0001" /></div>
          <div class="field"><label>R Measure</label><input id="rMeasure" type="number" step="0.0001" /></div>
          <div class="field"><label>Roll Offset</label><input id="rollOffsetDeg" type="number" step="0.001" /></div>
          <div class="field"><label>Pitch Offset</label><input id="pitchOffsetDeg" type="number" step="0.001" /></div>
          <div class="field"><label>Still Accel Threshold</label><input id="stillAccelThreshold" type="number" step="0.001" /></div>
          <div class="field"><label>Still Velocity Threshold</label><input id="stillVelocityThreshold" type="number" step="0.001" /></div>
          <div class="field"><label>Velocity Decay</label><input id="velocityDecay" type="number" step="0.001" min="0" max="1" /></div>
        </div>
        <div class="toolbar">
          <button class="btn-save" onclick="saveKalman()">Save to EEPROM</button>
          <button class="btn-secondary" onclick="reloadKalman()">Refresh</button>
        </div>
        <div class="note">These settings are stored in ESP32 EEPROM. Use smaller R for a more responsive filter, larger R for more smoothing. Q controls process noise; the offsets let you fine-tune the still attitude baseline.</div>
      </div>

      <div class="card">
        <div class="subhead">Live Log</div>
        <h2>Events</h2>
        <div class="log" id="log">Waiting for data...</div>
        <div class="footer">If raw motion still drifts at rest, raise the still thresholds or increase R slightly.</div>
      </div>
    </section>
  </div>

  <script>
    const history = [];
    const maxPoints = 240;
    const canvas = document.getElementById('plot');
    const ctx = canvas.getContext('2d');
    const logBox = document.getElementById('log');
    const seriesConfig = {
      rawRoll: { label: 'Raw Roll', color: '#f97316' },
      rawPitch: { label: 'Raw Pitch', color: '#facc15' },
      rawYaw: { label: 'Raw Yaw', color: '#fb7185' },
      rawDistance: { label: 'Raw Distance', color: '#22d3ee' },
      kalmanRoll: { label: 'Kalman Roll', color: '#4ade80' },
      kalmanPitch: { label: 'Kalman Pitch', color: '#60a5fa' },
      kalmanYaw: { label: 'Kalman Yaw', color: '#c084fc' },
      kalmanDistance: { label: 'Kalman Distance', color: '#e879f9' }
    };

    function resizeCanvas() {
      const rect = canvas.getBoundingClientRect();
      const ratio = window.devicePixelRatio || 1;
      canvas.width = rect.width * ratio;
      canvas.height = rect.height * ratio;
      ctx.setTransform(ratio, 0, 0, ratio, 0, 0);
      drawGraph();
    }

    window.addEventListener('resize', resizeCanvas);

    function appendLog(message) {
      const time = new Date().toLocaleTimeString();
      logBox.textContent = `[${time}] ${message}\n` + logBox.textContent;
    }

    function setText(id, value) {
      const node = document.getElementById(id);
      if (node) {
        node.textContent = value;
      }
    }

    function updateMetrics(data) {
      setText('runState', data.running ? 'Running' : 'Idle');
      setText('wifiMode', data.wifiMode || '---');
      setText('ssid', data.ssid || '---');
      setText('ip', data.ip || '---');
      setText('leftTicks', data.leftTicks ?? 0);
      setText('rightTicks', data.rightTicks ?? 0);
      setText('encDist', Number(data.encoderDistanceM || 0).toFixed(3));
      setText('targetDist', Number(data.targetDistanceM || 1).toFixed(3));

      setText('rawRoll', Number(data.rawRollDeg || 0).toFixed(2));
      setText('rawPitch', Number(data.rawPitchDeg || 0).toFixed(2));
      setText('rawYaw', Number(data.rawYawDeg || 0).toFixed(2));
      setText('rawDistance', Number(data.rawDistanceM || 0).toFixed(3));
      setText('rawVelocity', Number(data.rawVelocityMps || 0).toFixed(3));
      setText('rawAccel', Number(data.rawAccelMps2 || 0).toFixed(3));

      setText('kalmanRoll', Number(data.kalmanRollDeg || 0).toFixed(2));
      setText('kalmanPitch', Number(data.kalmanPitchDeg || 0).toFixed(2));
      setText('kalmanYaw', Number(data.kalmanYawDeg || 0).toFixed(2));
      setText('kalmanDistance', Number(data.kalmanDistanceM || 0).toFixed(3));
      setText('kalmanVelocity', Number(data.kalmanVelocityMps || 0).toFixed(3));
      setText('kalmanAccel', Number(data.kalmanAccelMps2 || 0).toFixed(3));

      ['qAngle', 'qBias', 'rMeasure', 'rollOffsetDeg', 'pitchOffsetDeg', 'stillAccelThreshold', 'stillVelocityThreshold', 'velocityDecay'].forEach((id) => {
        if (document.getElementById(id) && data[id] !== undefined) {
          document.getElementById(id).value = data[id];
        }
      });

      history.push({
        t: Date.now(),
        rawRoll: Number(data.rawRollDeg || 0),
        rawPitch: Number(data.rawPitchDeg || 0),
        rawYaw: Number(data.rawYawDeg || 0),
        rawDistance: Number(data.rawDistanceM || 0),
        kalmanRoll: Number(data.kalmanRollDeg || 0),
        kalmanPitch: Number(data.kalmanPitchDeg || 0),
        kalmanYaw: Number(data.kalmanYawDeg || 0),
        kalmanDistance: Number(data.kalmanDistanceM || 0)
      });

      if (history.length > maxPoints) {
        history.shift();
      }

      drawGraph();
    }

    function selectedSeries() {
      return Object.keys(seriesConfig).filter((key) => document.getElementById(`series-${key}`).checked);
    }

    function drawGrid(width, height, left, top, plotWidth, plotHeight) {
      ctx.strokeStyle = 'rgba(148, 163, 184, 0.12)';
      ctx.lineWidth = 1;
      for (let i = 0; i <= 5; i++) {
        const y = top + (plotHeight / 5) * i;
        ctx.beginPath();
        ctx.moveTo(left, y);
        ctx.lineTo(left + plotWidth, y);
        ctx.stroke();
      }
      for (let i = 0; i <= 6; i++) {
        const x = left + (plotWidth / 6) * i;
        ctx.beginPath();
        ctx.moveTo(x, top);
        ctx.lineTo(x, top + plotHeight);
        ctx.stroke();
      }
    }

    function drawGraph() {
      const width = canvas.clientWidth;
      const height = canvas.clientHeight;
      ctx.clearRect(0, 0, width, height);

      const active = selectedSeries();
      const left = 52;
      const top = 18;
      const plotWidth = width - 74;
      const plotHeight = height - 44;

      const legend = document.getElementById('legend');
      legend.innerHTML = '';

      if (!active.length || !history.length) {
        ctx.fillStyle = '#93a4c7';
        ctx.font = '14px ui-sans-serif, system-ui, sans-serif';
        ctx.fillText('Select one or more series to plot.', left, top + 30);
        return;
      }

      const values = [];
      active.forEach((seriesKey) => {
        history.forEach((point) => values.push(point[seriesKey]));
      });

      let min = Math.min(...values);
      let max = Math.max(...values);
      if (Math.abs(max - min) < 0.001) {
        max += 1;
        min -= 1;
      }
      const pad = (max - min) * 0.12;
      min -= pad;
      max += pad;

      drawGrid(width, height, left, top, plotWidth, plotHeight);

      ctx.fillStyle = '#93a4c7';
      ctx.font = '11px ui-sans-serif, system-ui, sans-serif';
      ctx.fillText(max.toFixed(2), 8, top + 10);
      ctx.fillText(min.toFixed(2), 8, top + plotHeight - 2);

      const xStep = plotWidth / Math.max(history.length - 1, 1);

      active.forEach((seriesKey) => {
        const cfg = seriesConfig[seriesKey];
        ctx.beginPath();
        history.forEach((point, index) => {
          const value = point[seriesKey];
          const x = left + index * xStep;
          const y = top + plotHeight - ((value - min) / (max - min)) * plotHeight;
          if (index === 0) {
            ctx.moveTo(x, y);
          } else {
            ctx.lineTo(x, y);
          }
        });
        ctx.strokeStyle = cfg.color;
        ctx.lineWidth = 2.2;
        ctx.stroke();

        const item = document.createElement('span');
        item.innerHTML = `<i style="background:${cfg.color}"></i>${cfg.label}`;
        legend.appendChild(item);
      });
    }

    async function sendAction(path) {
      const response = await fetch(path, { method: 'POST' });
      const data = await response.json();
      appendLog(data.message || 'ok');
      await pollOnce();
    }

    async function loadKalman() {
      const response = await fetch('/api/kalman', { cache: 'no-store' });
      const data = await response.json();
      ['qAngle', 'qBias', 'rMeasure', 'rollOffsetDeg', 'pitchOffsetDeg', 'stillAccelThreshold', 'stillVelocityThreshold', 'velocityDecay'].forEach((id) => {
        if (document.getElementById(id) && data[id] !== undefined) {
          document.getElementById(id).value = data[id];
        }
      });
      appendLog('Kalman settings loaded from EEPROM.');
    }

    async function reloadKalman() {
      await loadKalman();
      await pollOnce();
    }

    async function saveKalman() {
      const params = new URLSearchParams();
      ['qAngle', 'qBias', 'rMeasure', 'rollOffsetDeg', 'pitchOffsetDeg', 'stillAccelThreshold', 'stillVelocityThreshold', 'velocityDecay'].forEach((id) => {
        params.set(id, document.getElementById(id).value);
      });

      const response = await fetch(`/api/kalman?${params.toString()}`, { method: 'POST' });
      const data = await response.json();
      appendLog('Kalman settings saved to EEPROM.');
      ['qAngle', 'qBias', 'rMeasure', 'rollOffsetDeg', 'pitchOffsetDeg', 'stillAccelThreshold', 'stillVelocityThreshold', 'velocityDecay'].forEach((id) => {
        if (document.getElementById(id) && data[id] !== undefined) {
          document.getElementById(id).value = data[id];
        }
      });
      await pollOnce();
    }

    async function startCalibration(seconds) {
      const response = await fetch(`/api/calibrate?duration=${seconds}`, { method: 'POST' });
      const data = await response.json();
      appendLog(data.message || 'calibration complete');
      await pollOnce();
    }

    async function pollOnce() {
      const response = await fetch('/api/status', { cache: 'no-store' });
      const data = await response.json();
      updateMetrics(data);
    }

    document.querySelectorAll('input[type="checkbox"]').forEach((el) => {
      el.addEventListener('change', drawGraph);
    });

    loadKalman();
    resizeCanvas();
    setInterval(pollOnce, 350);
    pollOnce();
  </script>
</body>
</html>
)HTML";
