#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DNSServer.h>

const int LED_PIN = 2;
WebServer server(80);
DNSServer dnsServer;
Preferences preferences;

String ssid = "";
String password = "";
String networksHTML = ""; // Cache for scan results

// Forward declaration
void handleNotFound(); 

// Helper to wrap content in a professional HTML structure
String getHTML(String title, String content, String script = "", bool showLang = true) {
  String html = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>" + title + "</title>";
  html += "<style>";
  html += ":root { --primary: #2563eb; --bg: #f3f4f6; --card: #ffffff; --text: #1f2937; }";
  html += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; background: var(--bg); color: var(--text); display: flex; align-items: center; justify-content: center; min-height: 100vh; margin: 0; padding: 20px; box-sizing: border-box; }";
  html += ".card { background: var(--card); padding: 2rem; border-radius: 1rem; box-shadow: 0 10px 15px -3px rgba(0,0,0,0.1); width: 100%; max-width: 400px; text-align: center; }";
  html += "h1 { margin-top: 0; color: var(--primary); font-size: 1.5rem; margin-bottom: 1.5rem; }";
  html += "input, select { width: 100%; padding: 0.75rem; margin-top: 0.5rem; margin-bottom: 1rem; border: 1px solid #d1d5db; border-radius: 0.5rem; box-sizing: border-box; font-size: 1rem; transition: border-color 0.2s; background: white; }";
  html += "input:focus, select:focus { outline: none; border-color: var(--primary); ring: 2px solid rgba(37, 99, 235, 0.2); }";
  html += "label { font-weight: 600; font-size: 0.875rem; display: block; text-align: left; color: #4b5563; }";
  html += "button { width: 100%; padding: 0.75rem; background: var(--primary); color: white; border: none; border-radius: 0.5rem; font-size: 1rem; font-weight: 600; cursor: pointer; transition: background 0.2s; margin-top: 1rem; }";
  html += "button:hover { background: #1d4ed8; }";
  html += ".hidden { display: none; }";
  html += ".status { margin-top: 1rem; color: #6b7280; font-size: 0.9rem; }";
  html += ".spinner { border: 3px solid #f3f3f3; border-top: 3px solid var(--primary); border-radius: 50%; width: 24px; height: 24px; animation: spin 1s linear infinite; margin: 1rem auto; }";
  html += "@keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }";
  html += ".lang-select { margin-bottom: 1rem; width: 100%; margin-top: 0.5rem; }";
  html += ".lang-container { text-align: left; margin-bottom: 1rem; display: " + String(showLang ? "block" : "none") + "; }";
  html += ".password-container { position: relative; width: 100%; margin-top: 0.5rem; margin-bottom: 1rem; }";
  html += ".password-container input { margin: 0 !important; }";
  html += ".toggle-pass { position: absolute; right: 12px; top: 50%; transform: translateY(-50%); cursor: pointer; font-size: 1.2rem; user-select: none; line-height: 1; }";
  html += "</style></head><body>";
  html += "<div class='card'>";
  html += "<div class='lang-container'>";
  html += "<label for='lang' data-i18n='l_lang' style='font-size: 0.75rem; color: #6b7280;'>Language</label>";
  html += "<select id='lang' class='lang-select' onchange='setLang(this.value)'><option value='en'>🇺🇸 English</option><option value='es'>🇪🇸 Español</option><option value='zh'>🇨🇳 中文 (Mandarin)</option><option value='pt'>🇧🇷 Português</option><option value='fr'>🇫🇷 Français</option></select>";
  html += "</div>";
  html += content + "</div>";
  
  html += "<script>\n";
  html += "const dict = {\n";
  html += "  \"en\": {\"t_cfg\": \"WiFi Config\", \"l_ssid\": \"Select Network (SSID)\", \"l_pass\": \"Password\", \"b_save\": \"Save & Connect\", \"l_rescan\": \"Rescan (Restarts AP)\", \"t_saving\": \"Saving...\", \"msg_app\": \"Applying changes & restarting...\", \"t_saved\": \"Saved!\", \"msg_cred\": \"Credentials updated.\", \"msg_rest\": \"Device is restarting...\", \"t_err\": \"Error\", \"msg_miss\": \"Missing required fields.\", \"b_retry\": \"Try Again\", \"opt_no\": \"No networks found\", \"opt_sel\": \"Select a network...\", \"t_scan\": \"Scanning...\", \"msg_scan\": \"Restarting to scan networks. Reconnect in 10s.\", \"l_lang\": \"Language\"},\n";
  html += "  \"es\": {\"t_cfg\": \"Configuración WiFi\", \"l_ssid\": \"Seleccionar Red (SSID)\", \"l_pass\": \"Contraseña\", \"b_save\": \"Guardar y Conectar\", \"l_rescan\": \"Escanear de nuevo (Reinicia AP)\", \"t_saving\": \"Guardando...\", \"msg_app\": \"Aplicando cambios y reiniciando...\", \"t_saved\": \"¡Guardado!\", \"msg_cred\": \"Credenciales actualizadas.\", \"msg_rest\": \"El dispositivo se está reiniciando...\", \"t_err\": \"Error\", \"msg_miss\": \"Faltan campos requeridos.\", \"b_retry\": \"Intentar de nuevo\", \"opt_no\": \"No se encontraron redes\", \"opt_sel\": \"Selecciona una red...\", \"t_scan\": \"Escaneando...\", \"msg_scan\": \"Reiniciando para escanear. Reconecte en 10s.\", \"l_lang\": \"Lenguaje\"},\n";
  html += "  \"zh\": {\"t_cfg\": \"WiFi 配置\", \"l_ssid\": \"选择网络 (SSID)\", \"l_pass\": \"密码\", \"b_save\": \"保存并连接\", \"l_rescan\": \"重新扫描 (重启 AP)\", \"t_saving\": \"保存中...\", \"msg_app\": \"正在应用更改并重启...\", \"t_saved\": \"已保存！\", \"msg_cred\": \"凭据已更新。\", \"msg_rest\": \"设备正在重启...\", \"t_err\": \"错误\", \"msg_miss\": \"缺少必填字段。\", \"b_retry\": \"重试\", \"opt_no\": \"未找到网络\", \"opt_sel\": \"请选择网络...\", \"t_scan\": \"扫描中...\", \"msg_scan\": \"正在重启以扫描网络。请在10秒后重新连接。\", \"l_lang\": \"语言\"},\n";
  html += "  \"pt\": {\"t_cfg\": \"Configuração WiFi\", \"l_ssid\": \"Selecionar Rede (SSID)\", \"l_pass\": \"Senha\", \"b_save\": \"Salvar e Conectar\", \"l_rescan\": \"Escanear novamente (Reinicia AP)\", \"t_saving\": \"Salvando...\", \"msg_app\": \"Aplicando alterações e reiniciando...\", \"t_saved\": \"Salvo!\", \"msg_cred\": \"Credenciais atualizadas.\", \"msg_rest\": \"O dispositivo está reiniciando...\", \"t_err\": \"Erro\", \"msg_miss\": \"Campos obrigatórios ausentes.\", \"b_retry\": \"Tentar novamente\", \"opt_no\": \"Nenhuma rede encontrada\", \"opt_sel\": \"Selecione uma rede...\", \"t_scan\": \"Escaneando...\", \"msg_scan\": \"Reiniciando para escanear. Reconecte in 10s.\", \"l_lang\": \"Idioma\"},\n";
  html += "  \"fr\": {\"t_cfg\": \"Configuration WiFi\", \"l_ssid\": \"Sélectionner Réseau (SSID)\", \"l_pass\": \"Mot de passe\", \"b_save\": \"Enregistrer et Connecter\", \"l_rescan\": \"Scanner à nouveau (Redémarre AP)\", \"t_saving\": \"Enregistrement...\", \"msg_app\": \"Application des modifications...\", \"t_saved\": \"Enregistré !\", \"msg_cred\": \"Identifiants mis à jour.\", \"msg_rest\": \"Redémarrage de l'appareil...\", \"t_err\": \"Error\", \"msg_miss\": \"Champs requis manquants.\", \"b_retry\": \"Réessayer\", \"opt_no\": \"Aucun réseau trouvé\", \"opt_sel\": \"Sélectionnez un réseau...\", \"t_scan\": \"Scan en cours...\", \"msg_scan\": \"Redémarrage pour scanner. Reconnexion dans 10s.\", \"l_lang\": \"Langue\"}\n";
  html += "};\n";
  
  html += "function setLang(l){";
  html += " localStorage.setItem('lang',l);";
  html += " const t=dict[l]||dict['en'];";
  html += " document.querySelectorAll('[data-i18n]').forEach(e=>{";
  html += "  const k=e.getAttribute('data-i18n');";
  html += "  if(t[k]) {";
  html += "   if(e.tagName=='INPUT') e.placeholder=t[k];";
  html += "   else e.innerHTML=t[k];";
  html += "  }";
  html += " });";
  html += "}";
  html += "function togglePass(){";
  html += " const p=document.getElementById('pass');";
  html += " const s=document.getElementById('toggleIcon');";
  html += " if(p.type=='password'){ p.type='text'; s.innerHTML='🙈'; }";
  html += " else { p.type='password'; s.innerHTML='👁️'; }";
  html += "}";
  
  html += "window.addEventListener('DOMContentLoaded', ()=>{";
  html += " const l=localStorage.getItem('lang')||'en';";
  html += " document.getElementById('lang').value=l;";
  html += " setLang(l);";
  html += "});";
  html += script;
  html += "</script>";
  html += "</body></html>";
  return html;
}

// Perform scan securely properly before AP starts
void performScan() {
  Serial.println("Scanning networks...");
  // Temporarily set STA mode to scan robustly
  WiFi.mode(WIFI_STA); 
  WiFi.disconnect();
  delay(100);
  
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  
  if (n == 0) {
    networksHTML = "<option value='' disabled data-i18n='opt_no'>No se encontraron redes</option>";
  } else {
    networksHTML = "<option value='' disabled selected data-i18n='opt_sel'>Selecciona una red...</option>";
    for (int i = 0; i < n; ++i) {
      networksHTML += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + " dBm)</option>";
    }
  }
}

// Function to serve the HTML configuration page
void handleRoot() {
  // Use cached networksHTML instead of scanning here logic to avoid connection drops
  String content = "<div id='mainForm'>";
  content += "<h1 data-i18n='t_cfg'>Configurar WiFi</h1>";
  content += "<form action='/save' method='POST' onsubmit='showLoading()'>";
  content += "<label data-i18n='l_ssid'>Seleccionar Red (SSID)</label>";
  content += "<select name='ssid' required>" + networksHTML + "</select>";
  content += "<label data-i18n='l_pass'>Contraseña</label>";
  content += "<div class='password-container'>";
  content += "<input type='password' id='pass' name='password' required placeholder='********'>";
  content += "<span class='toggle-pass' id='toggleIcon' onclick='togglePass()'>👁️</span>";
  content += "</div>";
  content += "<button type='submit' data-i18n='b_save'>Guardar y Conectar</button>";
  content += "</form>";
  content += "<p style='margin-top:10px;'><a href='/rescan' style='color:#6b7280;text-decoration:underline;font-size:0.9rem;' data-i18n='l_rescan'>Escanear de nuevo (Reinicia AP)</a></p>";
  content += "</div>";
  
  content += "<div id='loading' class='hidden'>";
  content += "<h1 data-i18n='t_saving'>Guardando...</h1>";
  content += "<div class='spinner'></div>";
  content += "<p class='status' data-i18n='msg_app'>Aplicando cambios y reiniciando...</p>";
  content += "</div>";

  String script = "function showLoading() { document.getElementById('mainForm').classList.add('hidden'); document.getElementById('loading').classList.remove('hidden'); }";
  
  server.send(200, "text/html", getHTML("Configuración WiFi", content, script));
}

// Handler to force a rescan (restarts AP to be safe)
void handleRescan() {
  String content = "<h1 data-i18n='t_scan'>Escaneando...</h1><p data-i18n='msg_scan'>El dispositivo se reiniciará para escanear redes. Vuelva a conectarse en 10 segundos.</p>";
  String script = "setTimeout(function(){window.location.href='/';}, 5000);";
  server.send(200, "text/html", getHTML("Escaneando", content, script));
  delay(1000);
  // Restarting is the easiest way to reset radios/scan cleanly without logic complexity
  ESP.restart(); 
}

// Function to handle form submission
void handleSave() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String new_ssid = server.arg("ssid");
    String new_password = server.arg("password");

    preferences.begin("wifi", false);
    preferences.putString("ssid", new_ssid);
    preferences.putString("password", new_password);
    preferences.end();
    
    String content = "<h1 style='color: #10b981;' data-i18n='t_saved'>¡Guardado!</h1>";
    content += "<p class='status' data-i18n='msg_cred'>Credenciales actualizadas correctamente.</p>";
    content += "<p class='status' data-i18n='msg_rest'>El dispositivo se está reiniciando...</p>";
    
    server.send(200, "text/html", getHTML("Guardado", content, "", false));
    
    delay(2000);
    ESP.restart();
  } else {
    String content = "<h1 style='color: #ef4444;' data-i18n='t_err'>Error</h1>";
    content += "<p class='status' data-i18n='msg_miss'>Faltan campos requeridos.</p>";
    content += "<button onclick='history.back()' data-i18n='b_retry'>Intentar de nuevo</button>";
    server.send(400, "text/html", getHTML("Error", content, "", false));
  }
}

void handleNotFound() {
  server.sendHeader("Location", "/", true); 
  server.send(302, "text/plain", "");
}

void setupWiFi() {
  preferences.begin("wifi", true); // Read-only mode
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();

  if (ssid == "") {
    Serial.println("No SSID stored. Starting AP mode.");
    startAP();
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to WiFi");
  
  // Wait for connection for up to 10 seconds
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.println("\nFailed to connect. Starting AP mode.");
    startAP();
  }
}

void startAP() {
  // Do the scan FIRST, while in STA mode (cleaner)
  performScan();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-Config", ""); // Open AP, no password
  Serial.println("AP Started: ESP32-Config");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Setup DNS Server to redirect all domains to local IP
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());
  
  digitalWrite(LED_PIN, LOW); // Ensure LED is OFF in AP mode

  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.on("/rescan", handleRescan);
  // Important: Catch-all handler for captive portal redirection
  server.onNotFound(handleNotFound); 
  server.begin();
}

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

const int RESET_PIN = 4; // Conectar PIN 4 a GND para resetear
unsigned long buttonPressStartTime = 0;
bool buttonPressed = false;

void setup() {
  // Disable Brownout Detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RESET_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW); // Start with LED OFF

  // Explicitly reset WiFi logic to ensure clean state
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_OFF);
  delay(100);

  setupWiFi();
}

void handleResetButton() {
  // Check for Factory Reset (Pin 4 to GND)
  if (digitalRead(RESET_PIN) == LOW) {
    if (!buttonPressed) {
      buttonPressed = true;
      buttonPressStartTime = millis();
      Serial.println("Boton de reset detectado...");
    }
    
    // Feedback visual inmediato: Apagar LED mientras se presiona
    digitalWrite(LED_PIN, LOW); 

    // If held for 3 seconds, reset WiFi credentials
    if (millis() - buttonPressStartTime > 3000) {
      Serial.println("\nResetting WiFi Credentials...");
      
      // Blink LED rapidly to indicate reset action
      for (int i = 0; i < 10; i++) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        delay(50);
      }
      digitalWrite(LED_PIN, LOW);

      preferences.begin("wifi", false);
      preferences.clear();
      preferences.end();
      
      Serial.println("Credentials cleared. Restarting...");
      delay(1000);
      ESP.restart();
    }
  } else {
    // If button is released before 3 seconds, reset state
    if (buttonPressed) {
      Serial.println("Reset cancelado (boton soltado).");
    }
    buttonPressed = false;
  }
}

void loop() {
  handleResetButton();

  if (WiFi.getMode() == WIFI_AP) {
    // Process DNS requests to hijack traffic
    dnsServer.processNextRequest();
    server.handleClient();
  }
  
  // LED Logic: Only show status if NOT currently pressing the reset button
  if (!buttonPressed) {
    if (WiFi.status() == WL_CONNECTED) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  }
}
