# ESP32 + MQTT + Homebridge 冷氣控制系統

整合 ESP32、EMQX MQTT Broker 和 Homebridge，實現智慧家居冷氣控制。

## 🏗️ 架構圖

```
HomeKit (iPhone/iPad)
        ↓
   Homebridge
        ↓
   MQTT Broker (EMQX)
        ↓
      ESP32
        ↓
    紅外線發送器
        ↓
    Panasonic 冷氣
```

## 📋 需求

### 硬體
- ESP32 開發板
- 紅外線發射器 (GPIO 4)
- Panasonic 冷氣機

### 軟體
- Docker (運行 EMQX)
- Homebridge
- Node.js

## 🚀 快速開始

### 1. 啟動 EMQX MQTT Broker

```bash
docker run -d --name emqx \
  -p 1883:1883 \
  -p 8083:8083 \
  -p 8084:8084 \
  -p 8883:8883 \
  -p 18083:18083 \
  --restart=unless-stopped \
  emqx/emqx:latest
```

**管理介面：** http://localhost:18083
- 預設帳號: `admin`
- 預設密碼: `public`

### 2. 獲取 Mac 的 IP 地址

```bash
ifconfig | grep "inet " | grep -v 127.0.0.1
```

⚠️ **重要：** 將 `src/main.cpp` 中的 `MQTT_SERVER` 改為您的 Mac IP：

```cpp
#define MQTT_SERVER "192.168.1.100"  // 改為您的 Mac IP
```

### 3. 編譯並上傳到 ESP32

```bash
pio run -t upload
```

### 4. 監視 ESP32 輸出

```bash
pio device monitor
```

應該看到：
```
✓ WiFi 已連接!
✓ MQTT 已連接!
✓ Web Server 已啟動
```

### 5. 測試 MQTT 通訊

開啟終端測試：

```bash
# 訂閱狀態主題
mosquitto_sub -h localhost -t "homebridge/ac/state"

# 發送開機指令（另開終端）
mosquitto_pub -h localhost -t "homebridge/ac/command" \
  -m '{"command":"ON"}'

# 設定溫度 24 度
mosquitto_pub -h localhost -t "homebridge/ac/temperature/set" \
  -m '{"temperature":24}'

# 設定冷氣模式
mosquitto_pub -h localhost -t "homebridge/ac/mode/set" \
  -m '{"mode":"cool"}'
```

## 🏠 Homebridge 設定

### 安裝 Homebridge MQTT Thing 外掛

```bash
npm install -g homebridge-mqttthing
```

### 配置 Homebridge

將 `homebridge-config.json` 的內容添加到您的 Homebridge 配置中：

**位置：** `~/.homebridge/config.json`

```json
{
  "accessories": [
    {
      "accessory": "mqttthing",
      "type": "thermostat",
      "name": "冷氣",
      "url": "mqtt://192.168.8.181:1883",
      "topics": {
        "getTargetHeatingCoolingState": "homebridge/ac/mode/state",
        "setTargetHeatingCoolingState": "homebridge/ac/mode/set",
        "getTargetTemperature": "homebridge/ac/temperature/state",
        "setTargetTemperature": "homebridge/ac/temperature/set",
        "getCurrentTemperature": "homebridge/ac/temperature/state"
      },
      "heatingCoolingStateValues": {
        "OFF": "off",
        "HEAT": "heat",
        "COOL": "cool",
        "AUTO": "auto"
      },
      "minTemperature": 16,
      "maxTemperature": 30
    }
  ]
}
```

### 重啟 Homebridge

```bash
# 如果使用 systemd
sudo systemctl restart homebridge

# 如果手動運行
homebridge
```

## 📡 MQTT 主題說明

| 主題 | 類型 | 說明 | 範例 |
|------|------|------|------|
| `homebridge/ac/command` | 訂閱 | 開關控制 | `{"command":"ON"}` |
| `homebridge/ac/state` | 發布 | 完整狀態 | `{"power":"ON","temperature":26,"mode":"cool"}` |
| `homebridge/ac/temperature/set` | 訂閱 | 設定溫度 | `{"temperature":24}` |
| `homebridge/ac/temperature/state` | 發布 | 當前溫度 | `{"temperature":26}` |
| `homebridge/ac/mode/set` | 訂閱 | 設定模式 | `{"mode":"cool"}` |
| `homebridge/ac/mode/state` | 發布 | 當前模式 | `{"mode":"cool"}` |

### 支援的模式
- `auto` - 自動
- `cool` - 冷氣
- `heat` - 暖氣
- `dry` - 除濕
- `fan` - 送風

## 🌐 Web 控制介面

直接使用瀏覽器訪問 ESP32 的 IP 地址（在序列監視器中顯示）

範例：`http://192.168.1.50`

## 🔧 故障排除

### ESP32 無法連接 MQTT

1. 確認 EMQX 正在運行：
   ```bash
   docker ps | grep emqx
   ```

2. 檢查 ESP32 能否 ping 通您的 Mac：
   ```bash
   ping 192.168.1.100
   ```

3. 確認防火牆未阻擋 1883 端口

### Homebridge 無法控制

1. 查看 Homebridge 日誌：
   ```bash
   tail -f ~/.homebridge/homebridge.log
   ```

2. 使用 EMQX 管理介面監控訊息：
   http://localhost:18083 → Monitoring → WebSocket

### MQTT 消息格式錯誤

確保 JSON 格式正確：
```bash
# ✓ 正確
mosquitto_pub -h localhost -t "homebridge/ac/command" -m '{"command":"ON"}'

# ✗ 錯誤（缺少引號）
mosquitto_pub -h localhost -t "homebridge/ac/command" -m '{command:ON}'
```

## 📊 EMQX Dashboard 監控

訪問 http://localhost:18083

**功能：**
- 即時查看連接的客戶端
- 監控 MQTT 訊息流量
- 訂閱/發布測試
- 查看主題統計

## 🔐 安全建議（生產環境）

1. **啟用 MQTT 認證**
   ```cpp
   #define MQTT_USER "esp32"
   #define MQTT_PASSWORD "your_secure_password"
   ```

2. **配置 EMQX 認證**（EMQX Dashboard → Authentication）

3. **使用 TLS/SSL**（端口 8883）

## 📝 授權

MIT License

## 🤝 貢獻

歡迎提交 Issue 和 Pull Request！
