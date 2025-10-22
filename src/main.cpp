/**
 * ESP32 紅外線學習與控制系統
 * 使用 IRremoteESP8266 庫 + MQTT 整合
 * 
 * 功能：
 * - 接收 Panasonic AC 遙控器信號
 * - 發送 Panasonic AC 控制指令
 * - WiFi 網頁控制界面
 * - MQTT 雙向通訊（支援 Homebridge）
 * - 學習和存儲遙控器指令
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Panasonic.h>
#include "config.h"  // 引入配置文件

// ==================== 全局對象 ====================
IRsend irsend(IR_TX_GPIO);
IRPanasonicAc ac(IR_TX_GPIO);
WebServer server(80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ==================== 冷氣狀態 ====================
struct ACState {
    bool power = false;
    uint8_t temperature = 26;
    uint8_t mode = 1;  // 0:Auto, 1:Cool, 2:Dry, 3:Fan, 4:Heat
    uint8_t fanSpeed = 0;  // 0:Auto, 1:Low, 2:Medium, 3:High
} acState;

// ==================== 函數前置宣告 ====================
void sendPanasonicAC(bool power, uint8_t temp, uint8_t mode, uint8_t fan);
void publishACState();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void connectMQTT();
void setupWiFi();

// ==================== HTML 網頁 ====================
const char* HTML_PAGE = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 紅外線控制器</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: Arial, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); padding: 20px; }
        .container { max-width: 800px; margin: 0 auto; background: white; border-radius: 15px; padding: 30px; box-shadow: 0 10px 30px rgba(0,0,0,0.3); }
        h1 { text-align: center; color: #333; margin-bottom: 30px; }
        .section { margin: 25px 0; padding: 20px; background: #f8f9fa; border-radius: 10px; }
        .section h3 { color: #667eea; margin-bottom: 15px; }
        .btn { background: #667eea; color: white; border: none; padding: 12px 24px; border-radius: 8px; cursor: pointer; margin: 5px; font-size: 14px; transition: all 0.3s; }
        .btn:hover { background: #5568d3; transform: translateY(-2px); box-shadow: 0 5px 15px rgba(102,126,234,0.4); }
        .btn-danger { background: #e74c3c; }
        .btn-danger:hover { background: #c0392b; }
        .btn-success { background: #27ae60; }
        .btn-success:hover { background: #229954; }
        .status { padding: 15px; background: #e8f5e9; border-left: 4px solid #4caf50; border-radius: 5px; margin: 10px 0; }
        .temp-control { display: flex; align-items: center; gap: 10px; margin: 15px 0; }
        .temp-value { font-size: 24px; font-weight: bold; color: #667eea; min-width: 60px; text-align: center; }
        .fan-control { display: flex; align-items: center; gap: 10px; margin: 15px 0; }
        .fan-value { font-size: 18px; font-weight: bold; color: #667eea; min-width: 80px; text-align: center; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🏠 ESP32 紅外線控制器</h1>
        
        <div class="section">
            <h3>❄️ Panasonic 冷氣控制</h3>
            <div>
                <button class="btn btn-success" onclick="sendCmd('power_on')">🟢 開機</button>
                <button class="btn btn-danger" onclick="sendCmd('power_off')">🔴 關機</button>
            </div>
            
            <div class="temp-control">
                <button class="btn" onclick="changeTemp(-1)">➖</button>
                <div class="temp-value" id="tempValue">26°C</div>
                <button class="btn" onclick="changeTemp(1)">➕</button>
            </div>
            
            <div>
                <button class="btn" onclick="setMode('cool')">❄️ 冷氣</button>
                <button class="btn" onclick="setMode('heat')">🔥 暖氣</button>
                <button class="btn" onclick="setMode('dry')">💧 除濕</button>
                <button class="btn" onclick="setMode('fan')">🌀 送風</button>
            </div>
            
            <div class="fan-control">
                <span style="color: #667eea; font-weight: bold;">風量:</span>
                <button class="btn" onclick="setFan('auto')">� 自動</button>
                <button class="btn" onclick="setFan('low')">💨 低</button>
                <button class="btn" onclick="setFan('medium')">💨� 中</button>
                <button class="btn" onclick="setFan('high')">💨💨� 高</button>
            </div>
        </div>
        
        <div class="section">
            <h3>📡 系統狀態</h3>
            <div id="status" class="status">系統運行中</div>
        </div>
    </div>
    
    <script>
        let currentTemp = 26;
        
        function updateStatus() {
            fetch('/status')
                .then(r => r.json())
                .then(data => {
                    document.getElementById('status').innerHTML = data.status;
                });
        }
        
        function sendCmd(cmd) {
            fetch('/send?cmd=' + cmd)
                .then(r => r.text())
                .then(data => alert(data));
        }
        
        function changeTemp(delta) {
            currentTemp = Math.max(16, Math.min(30, currentTemp + delta));
            document.getElementById('tempValue').textContent = currentTemp + '°C';
            fetch('/send?cmd=temp_' + currentTemp);
        }
        
        function setMode(mode) {
            fetch('/send?cmd=mode_' + mode)
                .then(r => r.text())
                .then(data => alert(data));
        }
        
        function setFan(fan) {
            fetch('/send?cmd=fan_' + fan)
                .then(r => r.text())
                .then(data => alert(data));
        }
        
        setInterval(updateStatus, 5000);
        updateStatus();
    </script>
</body>
</html>
)rawliteral";

// ==================== MQTT 回調函數 ====================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.printf("\n📨 MQTT 消息: %s\n", topic);
    
    // 將 payload 轉為字串
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.printf("內容: %s\n", message.c_str());
    
    // 解析 JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.printf("❌ JSON 解析失敗: %s\n", error.c_str());
        return;
    }
    
    // 處理控制指令
    if (strcmp(topic, TOPIC_COMMAND) == 0) {
        const char* cmd = doc["command"];
        
        if (strcmp(cmd, "ON") == 0) {
            acState.power = true;
            acState.mode = 1;  // 冷氣模式
            sendPanasonicAC(true, acState.temperature, acState.mode, acState.fanSpeed);
            publishACState();
        }
        else if (strcmp(cmd, "OFF") == 0) {
            acState.power = false;
            sendPanasonicAC(false, acState.temperature, acState.mode, acState.fanSpeed);
            publishACState();
        }
    }
    else if (strcmp(topic, TOPIC_TEMP_SET) == 0) {
        int temp = doc["temperature"];
        if (temp >= 16 && temp <= 30) {
            acState.temperature = temp;
            acState.mode = 1;  // 冷氣模式
            Serial.printf("🌡️ 設定溫度: %d°C\n", temp);
            
            // 如果冷氣是開啟狀態，發送紅外線
            if (acState.power) {
                sendPanasonicAC(true, acState.temperature, acState.mode, acState.fanSpeed);
                publishACState();
            } else {
                // 即使關機也更新狀態（不發送紅外線）
                publishACState();
            }
        }
    }
    else if (strcmp(topic, TOPIC_MODE_SET) == 0) {
        // 處理 Homebridge 的開關控制（透過模式設定）
        const char* mode = doc["mode"];
        
        // 判斷開關機
        if (strcmp(mode, "cool") == 0) {
            acState.power = true;
            acState.mode = 1;  // 冷氣模式
            Serial.println("🟢 收到開機指令 (冷氣模式)");
            Serial.printf("🔍 Debug: acState.power = %d\n", acState.power);
        } 
        else if (strcmp(mode, "off") == 0) {
            acState.power = false;
            acState.mode = 1;  // 保持冷氣模式設定
            Serial.println("🔴 收到關機指令");
            Serial.printf("🔍 Debug: acState.power = %d\n", acState.power);
        }
        else {
            // 其他模式都視為關機
            acState.power = false;
            Serial.printf("⚠️ 未知模式: %s，視為關機\n", mode);
        }
        
        // 發送紅外線指令
        sendPanasonicAC(acState.power, acState.temperature, acState.mode, acState.fanSpeed);
        publishACState();
    }
    else if (strcmp(topic, TOPIC_FAN_SET) == 0) {
        // 處理風量設定（從 thermostat 的 RotationSpeed）
        const char* fanSpeed = doc["fanSpeed"];
        
        if (strcmp(fanSpeed, "auto") == 0) acState.fanSpeed = 0;
        else if (strcmp(fanSpeed, "low") == 0) acState.fanSpeed = 1;
        else if (strcmp(fanSpeed, "medium") == 0) acState.fanSpeed = 2;
        else if (strcmp(fanSpeed, "high") == 0) acState.fanSpeed = 3;
        
        Serial.printf("💨 設定風量: %s\n", fanSpeed);
        
        // 如果冷氣是開啟狀態，發送紅外線
        if (acState.power) {
            sendPanasonicAC(true, acState.temperature, acState.mode, acState.fanSpeed);
            publishACState();
        } else {
            // 即使關機也更新狀態（不發送紅外線）
            publishACState();
        }
    }
    else if (strcmp(topic, TOPIC_FAN_ACTIVE_SET) == 0) {
        // 處理風扇啟用/停用（從 fanv2）
        bool active = doc["active"];
        acState.power = active;
        
        Serial.printf("💨 風扇 %s\n", active ? "啟用" : "停用");
        
        sendPanasonicAC(acState.power, acState.temperature, acState.mode, acState.fanSpeed);
        publishACState();
    }
    else if (strcmp(topic, TOPIC_FAN_SPEED_SET) == 0) {
        // 處理風速設定（從 fanv2 的 RotationSpeed）
        const char* fanSpeed = doc["fanSpeed"];
        
        if (strcmp(fanSpeed, "auto") == 0) acState.fanSpeed = 0;
        else if (strcmp(fanSpeed, "low") == 0) acState.fanSpeed = 1;
        else if (strcmp(fanSpeed, "medium") == 0) acState.fanSpeed = 2;
        else if (strcmp(fanSpeed, "high") == 0) acState.fanSpeed = 3;
        
        Serial.printf("💨💨 設定風速: %s\n", fanSpeed);
        
        // 如果冷氣是開啟狀態，發送紅外線
        if (acState.power) {
            sendPanasonicAC(true, acState.temperature, acState.mode, acState.fanSpeed);
            publishACState();
        } else {
            // 即使關機也更新狀態（不發送紅外線）
            publishACState();
        }
    }
}

// ==================== MQTT 連接 ====================
void connectMQTT() {
    int retries = 0;
    const int maxRetries = 3;
    
    while (!mqttClient.connected() && retries < maxRetries) {
        Serial.printf("🔌 連接 MQTT Broker... (嘗試 %d/%d)\n", retries + 1, maxRetries);
        
        // 生成唯一的客戶端 ID（加上時間戳避免衝突）
        String clientId = String(MQTT_CLIENT_ID) + "-" + String(millis());
        
        if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
            Serial.println("✓ MQTT 已連接!");
            
            // 訂閱控制主題
            mqttClient.subscribe(TOPIC_COMMAND);
            mqttClient.subscribe(TOPIC_TEMP_SET);
            mqttClient.subscribe(TOPIC_MODE_SET);
            mqttClient.subscribe(TOPIC_FAN_SET);
            mqttClient.subscribe(TOPIC_FAN_ACTIVE_SET);
            mqttClient.subscribe(TOPIC_FAN_SPEED_SET);
            
            Serial.println("✓ 已訂閱控制主題");
            
            // 發布初始狀態
            publishACState();
            
            retries = 0;  // 重置重試計數器
        } else {
            retries++;
            Serial.printf("❌ MQTT 連接失敗, rc=%d", mqttClient.state());
            
            // 顯示錯誤原因
            switch (mqttClient.state()) {
                case -4: Serial.println(" (連接超時)"); break;
                case -3: Serial.println(" (連接中斷)"); break;
                case -2: Serial.println(" (連接失敗)"); break;
                case -1: Serial.println(" (已斷開)"); break;
                case 1: Serial.println(" (協議版本錯誤)"); break;
                case 2: Serial.println(" (客戶端 ID 被拒)"); break;
                case 3: Serial.println(" (服務器不可用)"); break;
                case 4: Serial.println(" (用戶名/密碼錯誤)"); break;
                case 5: Serial.println(" (未授權)"); break;
                default: Serial.println(" (未知錯誤)"); break;
            }
            
            if (retries < maxRetries) {
                Serial.println("3秒後重試...");
                delay(3000);
            } else {
                Serial.println("⚠️ 達到最大重試次數，將在下次循環重試");
                delay(10000);  // 等待 10 秒後再重試
                retries = 0;
            }
        }
    }
}

// ==================== 發布 AC 狀態 ====================
void publishACState() {
    // 發布到主要狀態主題（給 thermostat 使用）
    JsonDocument doc;
    
    doc["power"] = acState.power ? "ON" : "OFF";
    doc["temperature"] = acState.temperature;
    
    const char* modeStr[] = {"auto", "cool", "dry", "fan", "heat"};
    doc["mode"] = modeStr[acState.mode];
    
    const char* fanStr[] = {"auto", "low", "medium", "high"};
    doc["fanSpeed"] = fanStr[acState.fanSpeed];
    
    String output;
    serializeJson(doc, output);
    
    mqttClient.publish(TOPIC_STATE, output.c_str(), true);  // retained message
    Serial.printf("📤 發布狀態: %s\n", output.c_str());
    
    // 發布到風扇狀態主題（給 fanv2 使用）
    JsonDocument fanDoc;
    fanDoc["active"] = acState.power;
    fanDoc["fanSpeed"] = fanStr[acState.fanSpeed];
    
    String fanOutput;
    serializeJson(fanDoc, fanOutput);
    
    mqttClient.publish(TOPIC_FAN_STATE, fanOutput.c_str(), true);
    Serial.printf("📤 發布風扇狀態: %s\n", fanOutput.c_str());
}

// ==================== WiFi 連接 ====================
void setupWiFi() {
    Serial.println("\n=== WiFi 連接 ===");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi 已連接!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
        Serial.println("\n✗ WiFi 連接失敗!");
    }
}

// ==================== Panasonic AC 控制 ====================
void sendPanasonicAC(bool power, uint8_t temp, uint8_t mode, uint8_t fan) {
    Serial.println("\n=== 發送 Panasonic AC 指令 ===");
    
    if (power) {
        ac.on();
        Serial.println("電源: ON");
    } else {
        ac.off();
        Serial.println("電源: OFF");
    }
    
    ac.setTemp(temp);
    Serial.printf("溫度: %d°C\n", temp);
    
    switch (mode) {
        case 0: ac.setMode(kPanasonicAcAuto); Serial.println("模式: 自動"); break;
        case 1: ac.setMode(kPanasonicAcCool); Serial.println("模式: 冷氣"); break;
        case 2: ac.setMode(kPanasonicAcDry); Serial.println("模式: 除濕"); break;
        case 3: ac.setMode(kPanasonicAcFan); Serial.println("模式: 送風"); break;
        case 4: ac.setMode(kPanasonicAcHeat); Serial.println("模式: 暖氣"); break;
    }
    
    switch (fan) {
        case 0: ac.setFan(kPanasonicAcFanAuto); Serial.println("風量: 自動"); break;
        case 1: ac.setFan(kPanasonicAcFanMin); Serial.println("風量: 低"); break;
        case 2: ac.setFan(kPanasonicAcFanMed); Serial.println("風量: 中"); break;
        case 3: ac.setFan(kPanasonicAcFanMax); Serial.println("風量: 高"); break;
    }
    
    ac.setSwingVertical(kPanasonicAcSwingVAuto);
    
    ac.send();
    Serial.println("✓ 指令已發送\n");
}

// ==================== Web Server 處理 ====================
void handleRoot() {
    server.send(200, "text/html", HTML_PAGE);
}

void handleStatus() {
    String json = "{\"status\":\"系統運行中\"}";
    server.send(200, "application/json", json);
}

void handleSend() {
    String cmd = server.arg("cmd");
    String response = "Unknown command";
    
    if (cmd == "power_on") {
        sendPanasonicAC(true, 26, 1, 0);
        publishACState();
        response = "✓ 開機 (26°C 冷氣 自動風量)";
    }
    else if (cmd == "power_off") {
        sendPanasonicAC(false, 26, 1, 0);
        publishACState();
        response = "✓ 關機";
    }
    else if (cmd.startsWith("temp_")) {
        int temp = cmd.substring(5).toInt();
        if (temp >= 16 && temp <= 30) {
            sendPanasonicAC(true, temp, 1, 0);
            publishACState();
            response = "✓ 設定溫度: " + String(temp) + "°C";
        }
    }
    else if (cmd == "mode_cool") {
        sendPanasonicAC(true, 26, 1, 0);
        publishACState();
        response = "✓ 冷氣模式";
    }
    else if (cmd == "mode_heat") {
        sendPanasonicAC(true, 26, 4, 0);
        publishACState();
        response = "✓ 暖氣模式";
    }
    else if (cmd == "mode_dry") {
        sendPanasonicAC(true, 26, 2, 0);
        publishACState();
        response = "✓ 除濕模式";
    }
    else if (cmd == "mode_fan") {
        sendPanasonicAC(true, 26, 3, 0);
        publishACState();
        response = "✓ 送風模式";
    }
    else if (cmd == "fan_auto" || cmd == "fan_low" || cmd == "fan_medium" || cmd == "fan_high") {
        uint8_t fan = 0;
        if (cmd == "fan_auto") fan = 0;
        else if (cmd == "fan_low") fan = 1;
        else if (cmd == "fan_medium") fan = 2;
        else if (cmd == "fan_high") fan = 3;
        
        acState.fanSpeed = fan;
        sendPanasonicAC(true, acState.temperature, 1, fan);
        publishACState();
        
        const char* fanStr[] = {"自動", "低", "中", "高"};
        response = "✓ 風量: " + String(fanStr[fan]);
    }
    
    server.send(200, "text/plain", response);
}

// ==================== 初始化 ====================
void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n╔═══════════════════════════════════════╗");
    Serial.println("║   ESP32 紅外線 + MQTT 控制系統 v3.0 ║");
    Serial.println("║   支援 Homebridge 整合              ║");
    Serial.println("╚═══════════════════════════════════════╝\n");
    
    // 初始化 LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    // 初始化 IR 發送器
    irsend.begin();
    Serial.printf("✓ IR 發送器: GPIO %d\n", IR_TX_GPIO);
    
    // 初始化 Panasonic AC
    ac.begin();
    ac.setModel(kPanasonicRkr);
    Serial.println("✓ Panasonic AC 控制器已初始化");
    
    // 連接 WiFi
    setupWiFi();
    
    // 設置 MQTT
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setKeepAlive(60);
    mqttClient.setBufferSize(512);
    Serial.printf("✓ MQTT Server: %s:%d\n", MQTT_SERVER, MQTT_PORT);
    
    // 連接 MQTT
    connectMQTT();
    
    // 啟動 Web Server
    server.on("/", handleRoot);
    server.on("/status", handleStatus);
    server.on("/send", handleSend);
    
    server.begin();
    Serial.println("✓ Web Server 已啟動");
    
    Serial.println("\n========================================");
    Serial.println("系統就緒!");
    Serial.printf("網頁控制: http://%s\n", WiFi.localIP().toString().c_str());
    Serial.printf("MQTT 主題: %s\n", TOPIC_COMMAND);
    Serial.println("========================================\n");
}

// ==================== 主循環 ====================
void loop() {
    // 確保 MQTT 連接
    if (!mqttClient.connected()) {
        Serial.println("⚠️ MQTT 連接中斷，重新連接...");
        connectMQTT();
    }
    
    // 處理 MQTT 訊息
    mqttClient.loop();
    
    // 處理 Web 請求
    server.handleClient();
    
    // 保持 WiFi 連接
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️ WiFi 連接中斷，重新連接...");
        setupWiFi();
    }
    
    delay(10);
}
