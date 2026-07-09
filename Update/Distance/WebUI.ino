// WebUI.ino — simplified dashboard: encoder + raw MPU only, live distance plot
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
    .wrap { max-width: 1180px; margin: 0 auto; padding: 22px; }
    .panel {
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 22px;
      box-shadow: var(--shadow);
      backdrop-filter: blur(14px);
      padding: 22px;
    }
    .hero { display: grid; grid-template-columns: 1.4fr 1fr; gap: 18px; margin-bottom: 18px; }
    .eyebrow { text-transform: uppercase; letter-spacing: 0.22em; font-size: 11px; color: var(--accent); margin-bottom: 10px; }
    h1, h2, h3, p { margin: 0; }
    h1 { font-size: clamp(26px, 4vw, 44px); line-height: 1.05; }
    .subtitle { margin-top: 14px; color: var(--muted); max-width: 70ch; line-height: 1.65; font-size: 14px; }
    .toolbar { display: flex; flex-wrap: wrap; gap: 12px; margin-top: 20px; }
    button {
      border: 0; border-radius: 14px; padding: 12px 16px; cursor: pointer;
      font-weight: 700; font-size: 14px; transition: transform 0.15s ease, opacity 0.15s ease;
      box-shadow: 0 12px 28px rgba(0, 0, 0, 0.22);
    }
    button:hover { transform: translateY(-1px); }
    .btn-start { background: linear-gradient(135deg, #4ade80, #86efac); color: #06101c; }
    .btn-stop  { background: linear-gradient(135deg, #fb7185, #fda4af); color: #06101c; }
    .btn-reset { background: linear-gradient(135deg, #38bdf8, #7dd3fc); color: #06101c; }
    .btn-secondary { background: rgba(255,255,255,0.05); color: var(--text); border: 1px solid var(--line); box-shadow: none; }
    .stats-grid { display: grid; grid-template-columns: repeat(2, minmax(0, 1fr)); gap: 12px; }
    .mini-card { background: var(--panel-2); border: 1px solid var(--line); border-radius: 18px; padding: 16px; min-height: 88px; }
    .mini-card .label { color: var(--muted); font-size: 12px; margin-bottom: 8px; }
    .mini-card .value { font-size: 22px; font-weight: 800; line-height: 1.1; }
    .status-row { display: flex; flex-wrap: wrap; gap: 10px; margin-top: 16px; }
    .pill {
      display: inline-flex; align-items: center; gap: 8px;
      padding: 9px 12px; border-radius: 999px;
      border: 1px solid var(--line); background: rgba(255,255,255,0.04);
      color: var(--text); font-size: 12px;
    }
    .pill .dot { width: 8px; height: 8px; border-radius: 50%; background: var(--accent-2); box-shadow: 0 0 0 4px rgba(74, 222, 128, 0.12); }
    .card { padding: 20px; background: var(--panel); border: 1px solid var(--line); border-radius: 22px; box-shadow: var(--shadow); backdrop-filter: blur(14px); margin-bottom: 18px; }
    .subhead { color: var(--muted); font-size: 12px; margin-bottom: 14px; text-transform: uppercase; letter-spacing: 0.16em; }
    .metric-list { display: grid; grid-template-columns: repeat(4, minmax(0, 1fr)); gap: 12px; }
    .metric { background: rgba(255,255,255,0.03); border: 1px solid var(--line); border-radius: 16px; padding: 14px; min-height: 78px; }
    .metric .k { font-size: 12px; color: var(--muted); margin-bottom: 8px; }
    .metric .v { font-size: 20px; font-weight: 800; }
    .metric .u { font-size: 12px; color: var(--muted); margin-left: 4px; font-weight: 600; }
    .canvas-shell { position: relative; height: 320px; border-radius: 18px; overflow: hidden; border: 1px solid var(--line); background: linear-gradient(180deg, rgba(8, 15, 28, 0.95), rgba(6, 12, 22, 0.95)); }
    canvas { width: 100%; height: 100%; display: block; }
    .legend { display: flex; flex-wrap: wrap; gap: 10px; margin-top: 12px; color: var(--muted); font-size: 12px; }
    .legend span { display: inline-flex; align-items: center; gap: 6px; }
    .legend i { width: 10px; height: 10px; border-radius: 50%; display: inline-block; }
    .log { margin-top: 12px; min-height: 140px; max-height: 200px; overflow: auto; padding: 14px; border-radius: 16px; border: 1px solid var(--line); background: rgba(3, 7, 18, 0.68); font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace; font-size: 12px; line-height: 1.5; color: #c7d2fe; white-space: pre-wrap; }
    .footer { margin-top: 10px; color: var(--muted); font-size: 12px; }
    @media (max-width: 900px) {
      .hero, .metric-list { grid-template-columns: repeat(2, minmax(0, 1fr)); }
    }
    @media (max-width: 560px) {
      .hero { grid-template-columns: 1fr; }
      .metric-list { grid-template-columns: 1fr; }
      .canvas-shell { height: 260px; }
    }
  </style>
</head>
<body>
  <div class="wrap">
    <section class="hero">
      <div class="panel">
        <div class="eyebrow">Advanced Car Distance Monitor</div>
        <h1>Encoder + MPU Dashboard</h1>
        <p class="subtitle">Sends Start to drive forward until the encoder average reaches 1 m, then stops. The dashboard streams live encoder ticks/distance, raw MPU roll/pitch/yaw, and a single distance chart comparing encoder vs IMU integration.</p>
        <div class="toolbar">
          <button class="btn-start" onclick="sendAction('/api/start')">Start Run</button>
          <button class="btn-stop"  onclick="sendAction('/api/stop')">Stop</button>
          <button class="btn-reset" onclick="sendAction('/api/reset')">Reset</button>
          <button class="btn-secondary" onclick="startCalibration(10)">Calibrate MPU (10s)</button>
        </div>
        <div class="status-row">
          <div class="pill"><span class="dot"></span><span id="runState">Idle</span></div>
          <div class="pill">WiFi: <span id="wifiMode">---</span></div>
          <div class="pill">SSID: <span id="ssid">---</span></div>
          <div class="pill">IP: <span id="ip">---</span></div>
          <div class="pill">Run time: <span id="motionTime">0</span> ms</div>
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

    <section class="card">
      <div class="subhead">Raw MPU6050</div>
      <h2 style="margin-bottom: 14px;">Attitude &amp; IMU integration</h2>
      <div class="metric-list">
        <div class="metric"><div class="k">Roll</div><div class="v"><span id="roll">0.00</span><span class="u">deg</span></div></div>
        <div class="metric"><div class="k">Pitch</div><div class="v"><span id="pitch">0.00</span><span class="u">deg</span></div></div>
        <div class="metric"><div class="k">Yaw</div><div class="v"><span id="yaw">0.00</span><span class="u">deg</span></div></div>
        <div class="metric"><div class="k">Accel</div><div class="v"><span id="imuAccel">0.000</span><span class="u">m/s²</span></div></div>
        <div class="metric"><div class="k">IMU Velocity</div><div class="v"><span id="imuVel">0.000</span><span class="u">m/s</span></div></div>
        <div class="metric"><div class="k">IMU Distance</div><div class="v"><span id="imuDist">0.000</span><span class="u">m</span></div></div>
        <div class="metric"><div class="k">Roll Offset</div><div class="v"><span id="rollOffset">0.00</span><span class="u">deg</span></div></div>
        <div class="metric"><div class="k">Pitch Offset</div><div class="v"><span id="pitchOffset">0.00</span><span class="u">deg</span></div></div>
      </div>
      <div class="footer">Yaw drifts over time — there's no magnetometer. Distance from the IMU alone is just naive accel integration; it will drift too. Use the encoder value as the trusted source.</div>
    </section>

    <section class="card">
      <div class="subhead">Live Plot</div>
      <h2 style="margin-bottom: 14px;">Distance over time</h2>
      <div class="canvas-shell"><canvas id="plot"></canvas></div>
      <div class="legend" id="legend"></div>
      <div class="footer">Encoder is the ground truth; the IMU line shows how well the accel integration matches on its own (it's expected to drift).</div>
    </section>

    <section class="card">
      <div class="subhead">Events</div>
      <h2 style="margin-bottom: 14px;">Live log</h2>
      <div class="log" id="log">Waiting for data...</div>
    </section>
  </div>

  <script>
    const history = [];
    const maxPoints = 240;
    const canvas = document.getElementById('plot');
    const ctx = canvas.getContext('2d');
    const logBox = document.getElementById('log');
    const seriesConfig = {
      encoder: { label: 'Encoder',  color: '#4ade80' },
      imu:     { label: 'IMU',      color: '#38bdf8' }
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
      if (node) node.textContent = value;
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
      setText('roll', Number(data.rollDeg || 0).toFixed(2));
      setText('pitch', Number(data.pitchDeg || 0).toFixed(2));
      setText('yaw', Number(data.yawDeg || 0).toFixed(2));
      setText('imuAccel', Number(data.imuAccelMps2 || 0).toFixed(3));
      setText('imuVel', Number(data.velocityMps || 0).toFixed(3));
      setText('imuDist', Number(data.imuDistanceM || 0).toFixed(3));
      setText('rollOffset', Number(data.rollOffsetDeg || 0).toFixed(2));
      setText('pitchOffset', Number(data.pitchOffsetDeg || 0).toFixed(2));
      setText('motionTime', data.motionTimeMs ?? 0);

      history.push({
        t: Date.now(),
        encoder: Number(data.encoderDistanceM || 0),
        imu:     Number(data.imuDistanceM     || 0)
      });
      if (history.length > maxPoints) history.shift();
      drawGraph();
    }

    function drawGrid(left, top, plotWidth, plotHeight) {
      ctx.strokeStyle = 'rgba(148, 163, 184, 0.12)';
      ctx.lineWidth = 1;
      for (let i = 0; i <= 5; i++) {
        const y = top + (plotHeight / 5) * i;
        ctx.beginPath(); ctx.moveTo(left, y); ctx.lineTo(left + plotWidth, y); ctx.stroke();
      }
      for (let i = 0; i <= 6; i++) {
        const x = left + (plotWidth / 6) * i;
        ctx.beginPath(); ctx.moveTo(x, top); ctx.lineTo(x, top + plotHeight); ctx.stroke();
      }
    }

    function drawGraph() {
      const width = canvas.clientWidth;
      const height = canvas.clientHeight;
      ctx.clearRect(0, 0, width, height);

      const left = 56, top = 18;
      const plotWidth = width - 78;
      const plotHeight = height - 44;

      const legend = document.getElementById('legend');
      legend.innerHTML = '';

      if (!history.length) {
        ctx.fillStyle = '#93a4c7';
        ctx.font = '14px ui-sans-serif, system-ui, sans-serif';
        ctx.fillText('Waiting for data...', left, top + 30);
        return;
      }

      const values = [];
      history.forEach((p) => { values.push(p.encoder, p.imu); });
      let min = Math.min(...values, 0);
      let max = Math.max(...values, 1);
      if (Math.abs(max - min) < 0.01) { max += 1; min -= 0.5; }
      const pad = (max - min) * 0.12;
      min -= pad; max += pad;

      drawGrid(left, top, plotWidth, plotHeight);
      ctx.fillStyle = '#93a4c7';
      ctx.font = '11px ui-sans-serif, system-ui, sans-serif';
      ctx.fillText(max.toFixed(2) + ' m', 8, top + 10);
      ctx.fillText(min.toFixed(2) + ' m', 8, top + plotHeight - 2);

      const xStep = plotWidth / Math.max(history.length - 1, 1);
      Object.keys(seriesConfig).forEach((key) => {
        const cfg = seriesConfig[key];
        ctx.beginPath();
        history.forEach((point, index) => {
          const value = point[key];
          const x = left + index * xStep;
          const y = top + plotHeight - ((value - min) / (max - min)) * plotHeight;
          if (index === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
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
      if (path === '/api/reset') history.length = 0;
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

    resizeCanvas();
    setInterval(pollOnce, 350);
    pollOnce();
  </script>
</body>
</html>
)HTML";
