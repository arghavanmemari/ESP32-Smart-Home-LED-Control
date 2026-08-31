#include <WiFi.h>
#include <WebServer.h>

/* =========================================================================
    4 LED Control project 
   - AP Mode: SSID = ESP32-LED , PASS = 12345678
   - Web UI:
       /        -> HTML page
       /toggle  -> toggle LED by index
       /status  -> JSON status (led states only)
   ========================================================================= */
 /*=================================
 team members:
 Alireza Montajab 402249064
 Erfan Feghhi 402249047
 Arghavan Memari 402249062
 =================================== */  

// ---------------- WiFi AP ----------------
const char* AP_SSID = "ESP32-LED";
const char* AP_PASS = "CHANGE_ME";

WebServer server(80);

// ---------------- LED control pins ----------------
const int LED_PINS[4] = {25, 26, 27, 14};
bool ledState[4] = {false, false, false, false};

// ---------------- HTML UI ----------------
const char MAIN_page[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="fa">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 LED Controller</title>
  <style>
    :root{
      --bg1:#0b1220;
      --bg2:#111b2f;
      --card:#0f1a33cc;
      --text:#eaf0ff;
      --muted:#a9b7e6;
      --good:#19d38a;
      --bad:#ff5c7a;
      --btn:#1b2a52;
      --btn2:#2b3c6f;
      --shadow: 0 10px 30px rgba(0,0,0,.25);
      --radius: 18px;
    }
    *{box-sizing:border-box}
    body{
      margin:0;
      font-family: ui-sans-serif, system-ui, -apple-system, "Segoe UI", Arial;
      direction: rtl;
      color: var(--text);
      background: radial-gradient(1200px 600px at 20% 10%, #1b2a52 0%, transparent 55%),
                  radial-gradient(900px 500px at 90% 30%, #2a1b52 0%, transparent 55%),
                  linear-gradient(160deg, var(--bg1), var(--bg2));
      min-height: 100vh;
    }
    .wrap{ max-width: 980px; margin: 0 auto; padding: 22px 16px 40px; }
    .topbar{
      position: sticky; top: 0; z-index: 10;
      backdrop-filter: blur(10px);
      background: rgba(10,16,30,.55);
      border: 1px solid rgba(255,255,255,.06);
      border-radius: var(--radius);
      padding: 14px 14px;
      box-shadow: var(--shadow);
      margin-bottom: 18px;
    }
    .title{ display:flex; align-items:center; justify-content:space-between; gap:10px; }
    h1{ margin:0; font-size: 18px; letter-spacing:.2px; }
    .sub{
      margin-top:6px; color: var(--muted); font-size: 12px;
      display:flex; gap:10px; flex-wrap:wrap; align-items:center;
    }
    .pill{
      border:1px solid rgba(255,255,255,.10);
      background: rgba(255,255,255,.06);
      padding:6px 10px;
      border-radius: 999px;
    }
    .grid{ display:grid; grid-template-columns: repeat(2, minmax(0,1fr)); gap: 14px; }
    @media (max-width:720px){ .grid{ grid-template-columns: 1fr; } }
    .card{
      background: var(--card);
      border:1px solid rgba(255,255,255,.08);
      border-radius: var(--radius);
      box-shadow: var(--shadow);
      padding: 14px;
      overflow:hidden;
      position:relative;
    }
    .card::before{
      content:"";
      position:absolute; inset:-1px;
      background: radial-gradient(500px 160px at 15% 0%, rgba(25,211,138,.18), transparent 60%),
                  radial-gradient(500px 160px at 85% 0%, rgba(255,92,122,.14), transparent 60%);
      pointer-events:none;
    }
    .row{
      position:relative;
      display:flex;
      justify-content:space-between;
      align-items:flex-start;
      gap:12px;
    }
    .name{ font-weight:800; font-size: 16px; }
    .state{
      margin-top:6px;
      font-size: 12px;
      color: var(--muted);
      display:flex;
      gap:8px;
      align-items:center;
    }
    .dot{
      width:10px;height:10px;border-radius:99px;
      background: var(--bad);
      box-shadow: 0 0 0 4px rgba(255,92,122,.12);
    }
    .dot.on{
      background: var(--good);
      box-shadow: 0 0 0 4px rgba(25,211,138,.12);
    }
    .btn{
      user-select:none;
      border:none;
      color: var(--text);
      background: linear-gradient(180deg, var(--btn), var(--btn2));
      border:1px solid rgba(255,255,255,.10);
      padding: 10px 12px;
      border-radius: 14px;
      cursor:pointer;
      box-shadow: 0 10px 18px rgba(0,0,0,.25);
      min-width: 110px;
      text-align:center;
      font-weight:800;
      transition: transform .08s ease, filter .15s ease;
      position:relative;
      z-index:2;
    }
    .btn:active{ transform: scale(.98); }
    .btn.loading{ opacity:.7; cursor:wait; }
    .btn small{ display:block; font-weight:600; color: rgba(255,255,255,.75); }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="topbar">
      <div class="title">
        <h1>کنترل ۴ LED </h1>
        <div class="pill" id="ip">IP: 192.168.4.1</div>
      </div>
      <div class="sub">
        <div class="pill">Refresh: هر ۱ ثانیه</div>
        <div class="pill" id="statusPill">وضعیت: در حال اتصال...</div>
      </div>
    </div>

    <div class="grid" id="cards"></div>
  </div>

<script>
const cardsEl = document.getElementById('cards');
const statusPill = document.getElementById('statusPill');

async function toggle(i, btn){
  try{
    btn.classList.add('loading');
    await fetch(`/toggle?i=${i}`, {cache:'no-store'});
    await refresh();
  }finally{
    btn.classList.remove('loading');
  }
}

function render(data){
  cardsEl.innerHTML = "";
  for(let k=0;k<4;k++){
    const isOn = !!data.led[k];

    const card = document.createElement('div');
    card.className = 'card';
    card.innerHTML = `
      <div class="row">
        <div>
          <div class="name">LED ${k+1}</div>
          <div class="state">
            <span class="dot ${isOn ? 'on' : ''}"></span>
            <span>${isOn ? 'روشن' : 'خاموش'}</span>
          </div>
        </div>
        <button class="btn" id="btn${k}">
          ${isOn ? 'خاموش کن' : 'روشن کن'}
          <small>Toggle</small>
        </button>
      </div>
    `;
    cardsEl.appendChild(card);

    const btn = document.getElementById(`btn${k}`);
    btn.addEventListener('click', ()=>toggle(k, btn));
  }
}

async function refresh(){
  try{
    const r = await fetch('/status', {cache:'no-store'});
    if(!r.ok) throw new Error('http ' + r.status);
    const j = await r.json();
    render(j);
    statusPill.textContent = "وضعیت: آنلاین ✅";
  }catch(e){
    statusPill.textContent = "وضعیت: قطع ❌";
    cardsEl.innerHTML = `<div class="card">ارتباط با ESP32 برقرار نیست. اتصال WiFi را چک کن.</div>`;
  }
}

refresh();
setInterval(refresh, 1000);
</script>
</body>
</html>
)rawliteral";

// ---------------- Helpers ----------------
void setLed(int idx, bool on) {
  ledState[idx] = on;
  digitalWrite(LED_PINS[idx], on ? HIGH : LOW);
}

// ---------------- Web Handlers ----------------
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", MAIN_page);
}

void handleToggle() {
  if (!server.hasArg("i")) {
    server.send(400, "text/plain", "missing i");
    return;
  }

  int i = server.arg("i").toInt();
  if (i < 0 || i > 3) {
    server.send(400, "text/plain", "bad i");
    return;
  }

  setLed(i, !ledState[i]);
  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  String json = "{";
  json += "\"led\":[" + String(ledState[0] ? 1 : 0) + "," + String(ledState[1] ? 1 : 0) + "," +
          String(ledState[2] ? 1 : 0) + "," + String(ledState[3] ? 1 : 0) + "]";
  json += "}";
  server.send(200, "application/json; charset=utf-8", json);
}

// ---------------- Setup & Loop ----------------
void setup() {
  Serial.begin(115200);
  delay(1200);

  Serial.println();
  Serial.println("BOOT OK - starting...");

  for (int i = 0; i < 4; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    setLed(i, false);
  }
  Serial.println("LED pins configured.");

  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(AP_SSID, AP_PASS);

  Serial.print("softAP ok? ");
  Serial.println(ok ? "YES" : "NO");

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/status", handleStatus);

  server.begin();
  Serial.println("Web server started.");
  Serial.println("Open: http://192.168.4.1");
}

void loop() {
  server.handleClient();
}
