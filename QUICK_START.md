# 🚀 快速設置指南

## ✅ 當前狀態

- [x] EMQX Docker 容器已啟動
- [x] ESP32 程式碼已更新（MQTT 整合）
- [x] MQTT Broker IP 已配置：192.168.3.1
- [x] 測試腳本已創建

## 📝 下一步操作

### 1️⃣ 編譯並上傳到 ESP32

```bash
cd /Users/funtuanhomemac/esp32-project/esp32-ir-arduino
pio run -t upload
```

### 2️⃣ 監視 ESP32 輸出

```bash
pio device monitor
```

**預期輸出：**
```
✓ WiFi 已連接!
IP: 192.168.x.x
✓ MQTT 已連接!
✓ 已訂閱控制主題
✓ Web Server 已啟動
系統就緒!
```

### 3️⃣ 安裝 mosquitto 客戶端（如未安裝）

```bash
brew install mosquitto
```

### 4️⃣ 測試 MQTT 通訊

**方法 A：使用互動式測試腳本**
```bash
./mqtt-test.sh
```

**方法 B：手動測試**
```bash
# 訂閱狀態（開一個終端）
mosquitto_sub -h localhost -t "homebridge/ac/#" -v

# 發送指令（另開終端）
mosquitto_pub -h localhost -t "homebridge/ac/command" -m '{"command":"ON"}'
```

### 5️⃣ 訪問 EMQX Dashboard

開啟瀏覽器：http://localhost:18083
- 帳號：`admin`
- 密碼：`public`

**可以查看：**
- 連接的客戶端（應該看到 ESP32_AC_Controller）
- 訂閱的主題
- 即時訊息流量

### 6️⃣ 配置 Homebridge

**安裝外掛：**
```bash
npm install -g homebridge-mqttthing
```

**編輯配置：**
```bash
nano ~/.homebridge/config.json
```

將 `homebridge-config.json` 的內容合併到您的配置中。

**重啟 Homebridge：**
```bash
# macOS service
brew services restart homebridge

# 或手動運行
homebridge
```

### 7️⃣ 在 HomeKit 中添加配件

1. 開啟「家庭」App
2. 點擊右上角「+」→「加入配件」
3. 選擇「更多選項」
4. 找到「冷氣」配件
5. 輸入 Homebridge 的配對碼

## 🧪 測試流程

### 測試 1：MQTT 通訊
```bash
# 訂閱
mosquitto_sub -h localhost -t "homebridge/ac/state"

# 發送開機（另開終端）
mosquitto_pub -h localhost -t "homebridge/ac/command" -m '{"command":"ON"}'
```

**預期結果：**
- ESP32 序列監視器顯示收到指令
- 紅外線發送器發送訊號
- 訂閱終端收到狀態更新

### 測試 2：溫度控制
```bash
mosquitto_pub -h localhost -t "homebridge/ac/temperature/set" -m '{"temperature":24}'
```

### 測試 3：模式切換
```bash
mosquitto_pub -h localhost -t "homebridge/ac/mode/set" -m '{"mode":"cool"}'
```

### 測試 4：Web 控制
瀏覽器訪問 ESP32 的 IP（在序列監視器中顯示）

### 測試 5：Homebridge 控制
在「家庭」App 中控制冷氣

## ⚠️ 常見問題

### ESP32 無法連接 MQTT

**解決方案：**
1. 確認 EMQX 正在運行：
   ```bash
   docker ps | grep emqx
   ```

2. 檢查 Mac IP 是否正確：
   ```bash
   ifconfig | grep "inet " | grep -v 127.0.0.1
   ```

3. 測試網路連通性（從 ESP32 到 Mac）

4. 檢查防火牆設定（允許 1883 端口）

### MQTT 訊息未收到

**解決方案：**
1. 在 EMQX Dashboard 查看客戶端連接狀態
2. 查看訂閱主題是否正確
3. 使用 mosquitto_sub 驗證訊息是否發布

### Homebridge 無法控制

**解決方案：**
1. 檢查 homebridge-mqttthing 是否安裝成功
2. 驗證 config.json 格式正確
3. 查看 Homebridge 日誌：
   ```bash
   tail -f ~/.homebridge/homebridge.log
   ```

## 📊 系統架構

```
┌─────────────────┐
│   HomeKit App   │
│  (iPhone/iPad)  │
└────────┬────────┘
         │
┌────────▼────────┐
│   Homebridge    │
│  MQTT Thing     │
└────────┬────────┘
         │
┌────────▼────────┐
│  EMQX Broker    │
│  (Docker)       │
│  Port: 1883     │
└────────┬────────┘
         │
┌────────▼────────┐
│     ESP32       │
│  MQTT Client    │
└────────┬────────┘
         │
┌────────▼────────┐
│  IR Transmitter │
│    (GPIO 4)     │
└────────┬────────┘
         │
┌────────▼────────┐
│ Panasonic AC    │
└─────────────────┘
```

## 🎯 MQTT 主題結構

```
homebridge/ac/
├── command          (訂閱) 開關控制
├── state            (發布) 完整狀態
├── temperature/
│   ├── set          (訂閱) 設定目標溫度
│   └── state        (發布) 當前溫度
└── mode/
    ├── set          (訂閱) 設定運轉模式
    └── state        (發布) 當前模式
```

## 🔐 進階配置（可選）

### 啟用 MQTT 認證

1. 在 EMQX Dashboard 設定用戶名/密碼
2. 更新 ESP32 程式碼：
   ```cpp
   #define MQTT_USER "esp32"
   #define MQTT_PASSWORD "your_password"
   ```
3. 更新 Homebridge config.json

### 使用 SSL/TLS

- 端口改為 8883
- 配置證書

## 📚 參考資源

- [EMQX 文檔](https://www.emqx.io/docs)
- [homebridge-mqttthing](https://github.com/arachnetech/homebridge-mqttthing)
- [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266)

## 🆘 獲取幫助

如遇到問題：
1. 查看 ESP32 序列監視器輸出
2. 檢查 EMQX Dashboard 的連接狀態
3. 查看 Homebridge 日誌
4. 使用 mosquitto_sub/pub 測試 MQTT 通訊

---

**祝您使用愉快！🎉**
