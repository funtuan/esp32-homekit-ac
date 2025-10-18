# 🏠 ESP32 紅外線冷氣控制系統

使用 ESP32 開發板透過紅外線控制 Panasonic 冷氣，整合 MQTT、Web 介面與 HomeKit (Homebridge) 的完整智慧家居解決方案。

[![Platform](https://img.shields.io/badge/platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework](https://img.shields.io/badge/framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

## ✨ 功能特色

- 🌡️ **溫度控制**: 16-30°C 精確溫度調節
- 🔄 **多種模式**: 冷氣、暖氣、除濕、送風四種運轉模式
- 💨 **風量調整**: 自動、低、中、高四段風量選擇
- 🌐 **Web 控制**: 美觀直覺的網頁控制界面
- 📡 **MQTT 整合**: 支援 MQTT 雙向通訊協定
- 🍎 **HomeKit 支援**: 透過 Homebridge 整合至 Apple HomeKit
- 💾 **狀態同步**: 即時狀態回報與同步
- 🔒 **安全配置**: 配置文件分離，保護敏感資訊

## 📋 系統架構

```
┌─────────────────┐
│  HomeKit (iOS)  │
│   iPhone/iPad   │
└────────┬────────┘
         │
┌────────▼────────┐
│   Homebridge    │
│    (Node.js)    │
└────────┬────────┘
         │
┌────────▼────────┐
│  MQTT Broker    │
│     (EMQX)      │
└────────┬────────┘
         │
┌────────▼────────┐
│      ESP32      │
│  (Arduino Core) │
└────────┬────────┘
         │
┌────────▼────────┐
│  IR Transmitter │
└────────┬────────┘
         │
┌────────▼────────┐
│ Panasonic A/C   │
└─────────────────┘
```

## 🛠️ 硬體需求

### 必要元件
- **ESP32 開發板** (ESP32-DevKitC 或相容板)
- **紅外線發射器 LED** (940nm IR LED)
- **電阻** (100-220Ω，限流用)
- **連接線** (杜邦線)
- **Panasonic 冷氣機** (支援紅外線遙控)

### 接線圖

```
ESP32                    IR LED
GPIO 4 ─────┬────────── LED (+)
            │
          [100Ω]
            │
GND ────────┴────────── LED (-)
```

### 建議規格
- ESP32 最低記憶體: 520KB RAM
- 最低 Flash: 4MB
- WiFi 支援: 2.4GHz 802.11 b/g/n

## 💻 軟體需求

### 開發環境
- [PlatformIO](https://platformio.org/) (建議) 或 Arduino IDE
- [Visual Studio Code](https://code.visualstudio.com/) (搭配 PlatformIO)
- Python 3.x (PlatformIO 依賴)

### 執行環境
- **MQTT Broker**: EMQX, Mosquitto 或其他相容 broker
- **Homebridge**: (可選) 用於 HomeKit 整合
- **Node.js**: v14.0.0 或更高版本 (Homebridge 需要)

### 相依函式庫
- `IRremoteESP8266` ^2.8.6 - 紅外線控制
- `PubSubClient` ^2.8.0 - MQTT 通訊
- `ArduinoJson` ^7.0.4 - JSON 資料處理

## 🚀 快速開始

### 1️⃣ 安裝開發工具

**macOS:**
```bash
# 安裝 Homebrew (如未安裝)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安裝 PlatformIO
brew install platformio

# 安裝 mosquitto 客戶端 (測試用)
brew install mosquitto
```

**Linux:**
```bash
# 安裝 PlatformIO
pip install -U platformio

# 安裝 mosquitto 客戶端
sudo apt-get install mosquitto-clients
```

### 2️⃣ 複製專案

```bash
git clone <repository-url>
cd esp32-ir-arduino
```

### 3️⃣ 配置設定

```bash
# 複製配置範本
cp src/config.h.example src/config.h

# 編輯配置文件
nano src/config.h
```

**填入您的配置：**
```cpp
// WiFi 設定
#define WIFI_SSID           "您的WiFi名稱"
#define WIFI_PASSWORD       "您的WiFi密碼"

// MQTT 設定
#define MQTT_SERVER         "192.168.1.100"  // 您的 MQTT Broker IP
#define MQTT_PORT           1883
#define MQTT_CLIENT_ID      "ESP32_AC_Controller"

// 如需 MQTT 認證，填入帳密
#define MQTT_USER           ""
#define MQTT_PASSWORD       ""
```

> ⚠️ **注意**: `config.h` 包含敏感資訊，已被 `.gitignore` 排除，不會上傳至 Git

### 4️⃣ 編譯與上傳

```bash
# 編譯專案
pio run

# 上傳到 ESP32
pio run -t upload

# 開啟序列埠監視器
pio device monitor
```

**預期輸出：**
```
🔌 正在連接 WiFi: YourSSID...
✓ WiFi 已連接!
📍 IP 地址: 192.168.1.50
🔌 連接 MQTT Broker...
✓ MQTT 已連接!
✓ 已訂閱主題: homebridge/ac/command
✓ 已訂閱主題: homebridge/ac/temperature/set
✓ 已訂閱主題: homebridge/ac/mode/set
✓ Web Server 已啟動: http://192.168.1.50
🎉 系統就緒!
```

### 5️⃣ 啟動 MQTT Broker

**使用 Docker (推薦):**
```bash
docker run -d --name emqx \
  -p 1883:1883 \
  -p 8083:8083 \
  -p 8883:8883 \
  -p 18083:18083 \
  --restart=unless-stopped \
  emqx/emqx:latest
```

**訪問管理介面**: http://localhost:18083
- 預設帳號: `admin`
- 預設密碼: `public`

### 6️⃣ 測試功能

**開啟瀏覽器控制:**
```
http://[ESP32的IP地址]
```

**使用 MQTT 測試:**
```bash
# 訂閱狀態主題
mosquitto_sub -h localhost -t "homebridge/ac/#" -v

# 發送開機指令 (另開終端)
mosquitto_pub -h localhost -t "homebridge/ac/command" \
  -m '{"command":"ON"}'

# 設定溫度 24 度
mosquitto_pub -h localhost -t "homebridge/ac/temperature/set" \
  -m '{"temperature":24}'

# 設定為冷氣模式
mosquitto_pub -h localhost -t "homebridge/ac/mode/set" \
  -m '{"mode":"cool"}'

# 設定風量為高
mosquitto_pub -h localhost -t "homebridge/ac/fan/set" \
  -m '{"fan":"high"}'

# 關機
mosquitto_pub -h localhost -t "homebridge/ac/command" \
  -m '{"command":"OFF"}'
```

## 🔌 MQTT 主題說明

### 訂閱主題 (ESP32 接收指令)

| 主題 | 說明 | 訊息格式 |
|------|------|----------|
| `homebridge/ac/command` | 電源控制 | `{"command":"ON"}` / `{"command":"OFF"}` |
| `homebridge/ac/temperature/set` | 設定溫度 | `{"temperature":24}` (16-30) |
| `homebridge/ac/mode/set` | 設定模式 | `{"mode":"cool/heat/dry/fan/auto"}` |
| `homebridge/ac/fan/set` | 設定風量 | `{"fan":"auto/low/medium/high"}` |

### 發佈主題 (ESP32 回報狀態)

| 主題 | 說明 | 訊息格式 |
|------|------|----------|
| `homebridge/ac/state` | 電源狀態 | `{"power":true/false}` |
| `homebridge/ac/temperature/state` | 當前溫度 | `{"temperature":24}` |
| `homebridge/ac/mode/state` | 當前模式 | `{"mode":"cool"}` |
| `homebridge/ac/fan/state` | 當前風量 | `{"fan":"high"}` |

## 🍎 HomeKit 整合

### 安裝 Homebridge

```bash
# 安裝 Homebridge
sudo npm install -g --unsafe-perm homebridge

# 安裝 MQTT 外掛
sudo npm install -g homebridge-mqttthing
```

### 配置 Homebridge

編輯 `~/.homebridge/config.json`，參考 `homebridge-config.json` 範例：

```json
{
  "accessories": [
    {
      "accessory": "mqttthing",
      "type": "heaterCooler",
      "name": "冷氣",
      "url": "mqtt://localhost:1883",
      "topics": {
        "getActive": "homebridge/ac/state",
        "setActive": "homebridge/ac/command",
        "getCurrentTemperature": "homebridge/ac/temperature/state",
        "setTargetTemperature": "homebridge/ac/temperature/set",
        "getCurrentHeaterCoolerState": "homebridge/ac/mode/state",
        "setTargetHeaterCoolerState": "homebridge/ac/mode/set"
      }
    }
  ]
}
```

### 啟動 Homebridge

```bash
# macOS service
brew services start homebridge

# 或手動執行
homebridge
```

### 加入 HomeKit

1. 開啟 iPhone/iPad「家庭」App
2. 點擊右上角「+」→「加入配件」
3. 選擇「更多選項」
4. 找到「冷氣」配件
5. 輸入 Homebridge 顯示的配對碼
6. 完成設定

## 📱 Web 控制介面

ESP32 提供完整的網頁控制界面，支援：

- ✅ 電源開關 (開機/關機)
- ✅ 溫度調整 (16-30°C)
- ✅ 模式切換 (冷氣/暖氣/除濕/送風)
- ✅ 風量調節 (自動/低/中/高)
- ✅ 即時狀態顯示

**訪問方式：**
1. 確保您的電腦/手機與 ESP32 在同一 WiFi 網路
2. 開啟瀏覽器輸入 ESP32 的 IP 地址
3. 使用網頁上的按鈕控制冷氣

**特色：**
- 📱 響應式設計，支援手機、平板、電腦
- 🎨 美觀的漸層背景與圓角按鈕
- ⚡ 即時狀態更新 (每 5 秒自動更新)
- 🔄 AJAX 異步請求，操作流暢

## 🔧 進階設定

### 修改 GPIO 引腳

編輯 `src/config.h`:
```cpp
#define IR_TX_GPIO          4  // 改為您使用的 GPIO
#define LED_BUILTIN         2  // 內建 LED 引腳
```

### 修改 MQTT 主題

```cpp
#define TOPIC_COMMAND       "your/custom/topic/command"
#define TOPIC_STATE         "your/custom/topic/state"
// ... 其他主題
```

### 調整冷氣設定

編輯 `src/main.cpp` 中的 `ACState` 結構：
```cpp
struct ACState {
    bool power = false;
    uint8_t temperature = 26;     // 預設溫度
    uint8_t mode = 1;             // 預設模式 (1:冷氣)
    uint8_t fanSpeed = 3;         // 預設風量 (3:高)
} acState;
```

### 序列埠通訊速率

編輯 `platformio.ini`:
```ini
monitor_speed = 115200
upload_speed = 115200
```

### 自訂上傳埠

```ini
upload_port = /dev/cu.usbserial-0001    # macOS
upload_port = /dev/ttyUSB0              # Linux
upload_port = COM3                      # Windows
```

## 🐛 故障排除

### WiFi 連線問題

**症狀**: ESP32 無法連接 WiFi
```
🔌 正在連接 WiFi: YourSSID...
.........
❌ WiFi 連接失敗
```

**解決方案**:
1. 檢查 `config.h` 中的 WiFi SSID 和密碼是否正確
2. 確認 WiFi 是 2.4GHz (ESP32 不支援 5GHz)
3. 檢查 WiFi 訊號強度
4. 嘗試靠近路由器重新測試

### MQTT 連線問題

**症狀**: 無法連接 MQTT Broker
```
🔌 連接 MQTT Broker...
❌ MQTT 連接失敗，5秒後重試...
```

**解決方案**:
1. 確認 MQTT Broker 已啟動
   ```bash
   docker ps | grep emqx
   ```
2. 檢查 IP 地址是否正確
   ```bash
   ifconfig | grep "inet "
   ```
3. 測試網路連通性
   ```bash
   ping [MQTT_SERVER_IP]
   ```
4. 確認防火牆未阻擋 1883 端口

### 紅外線發射問題

**症狀**: ESP32 正常運作但冷氣無反應

**解決方案**:
1. 檢查紅外線 LED 接線是否正確
2. 確認 LED 極性 (長腳為正極)
3. 檢查限流電阻值 (100-220Ω)
4. 使用手機相機測試 IR LED (相機可看到紫光)
5. 確認 GPIO 引腳設定正確
6. 縮短 ESP32 與冷氣機的距離 (建議 3 公尺內)

### 上傳失敗

**症狀**: 無法上傳程式到 ESP32
```
A fatal error occurred: Failed to connect to ESP32
```

**解決方案**:
1. 按住 ESP32 的 BOOT 按鈕，再按 RESET 按鈕
2. 檢查 USB 線材 (某些線材僅供電，不支援資料傳輸)
3. 確認序列埠位址正確
   ```bash
   # macOS/Linux
   ls /dev/cu.* /dev/tty*
   
   # Windows
   mode
   ```
4. 安裝 USB 驅動程式 (CP2102 或 CH340)
5. 降低上傳速率
   ```ini
   upload_speed = 115200  # 改為 115200 或更低
   ```

### 編譯錯誤

**症狀**: 找不到 `config.h`
```
fatal error: config.h: No such file or directory
```

**解決方案**:
```bash
cp src/config.h.example src/config.h
# 然後編輯 config.h 填入配置
```

### Homebridge 無法連接

**症狀**: HomeKit 中找不到冷氣配件

**解決方案**:
1. 確認 Homebridge 正在運行
   ```bash
   ps aux | grep homebridge
   ```
2. 檢查 MQTT broker 狀態
3. 檢視 Homebridge 日誌
   ```bash
   tail -f ~/.homebridge/homebridge.log
   ```
4. 確認配置檔案格式正確 (JSON 語法)
5. 重新啟動 Homebridge
   ```bash
   brew services restart homebridge
   ```

## 📚 API 參考

### Web API 端點

| 端點 | 方法 | 參數 | 說明 |
|------|------|------|------|
| `/` | GET | - | 主頁面 |
| `/status` | GET | - | 取得系統狀態 (JSON) |
| `/send` | GET | `cmd` | 發送控制指令 |

### 控制指令格式

**電源控制:**
- `power_on` - 開機
- `power_off` - 關機

**溫度控制:**
- `temp_16` ~ `temp_30` - 設定溫度 (16-30°C)

**模式控制:**
- `mode_auto` - 自動模式
- `mode_cool` - 冷氣模式
- `mode_heat` - 暖氣模式
- `mode_dry` - 除濕模式
- `mode_fan` - 送風模式

**風量控制:**
- `fan_auto` - 自動風量
- `fan_low` - 低風量
- `fan_medium` - 中風量
- `fan_high` - 高風量

**範例:**
```bash
curl "http://192.168.1.50/send?cmd=power_on"
curl "http://192.168.1.50/send?cmd=temp_24"
curl "http://192.168.1.50/send?cmd=mode_cool"
curl "http://192.168.1.50/send?cmd=fan_high"
```

## 📂 專案結構

```
esp32-ir-arduino/
├── src/
│   ├── main.cpp              # 主程式
│   ├── config.h              # 配置文件 (不上傳)
│   └── config.h.example      # 配置範本
├── platformio.ini            # PlatformIO 配置
├── homebridge-config.json    # Homebridge 配置範例
├── mqtt-test.sh              # MQTT 測試腳本
├── README.md                 # 本文件
├── README_MQTT.md            # MQTT 詳細說明
├── QUICK_START.md            # 快速開始指南
├── CONFIG_SETUP.md           # 配置設定說明
└── .gitignore                # Git 忽略規則
```

## 🔒 安全性說明

### 配置文件保護

- ✅ `src/config.h` 包含敏感資訊，已被 `.gitignore` 排除
- ✅ 使用 `config.h.example` 作為範本
- ✅ 絕不將真實密碼上傳至 Git

### MQTT 安全建議

1. **使用認證**: 在 MQTT Broker 設定用戶名和密碼
2. **使用 TLS/SSL**: 啟用加密連線 (Port 8883)
3. **網路隔離**: 將 IoT 裝置放在獨立 VLAN
4. **定期更新**: 保持韌體和函式庫為最新版本

### WiFi 安全建議

1. 使用 WPA2 或 WPA3 加密
2. 設定強密碼
3. 停用 WPS 功能
4. 定期更換 WiFi 密碼

## 🤝 貢獻

歡迎提交 Issue 和 Pull Request！

### 開發流程

1. Fork 本專案
2. 建立您的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交您的修改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 開啟 Pull Request

### 程式碼規範

- 使用 4 空格縮排
- 函數和變數使用駝峰命名法
- 加入適當的註解
- 遵循現有的程式碼風格

## 📝 更新日誌

### v1.0.0 (2025-10-19)
- ✨ 初始版本發布
- ✨ 支援 Panasonic 冷氣紅外線控制
- ✨ Web 控制界面
- ✨ MQTT 整合
- ✨ HomeKit (Homebridge) 支援
- ✨ 即時狀態同步

## 📄 授權

本專案採用 MIT 授權條款 - 詳見 [LICENSE](LICENSE) 文件

## 🙏 致謝

- [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266) - 優秀的紅外線控制庫
- [PubSubClient](https://github.com/knolleary/pubsubclient) - MQTT 客戶端庫
- [ArduinoJson](https://arduinojson.org/) - JSON 處理庫
- [EMQX](https://www.emqx.io/) - 強大的 MQTT Broker
- [Homebridge](https://homebridge.io/) - HomeKit 整合平台

## 📞 聯絡方式

如有問題或建議，歡迎：
- 提交 [Issue](../../issues)
- 發送 Pull Request
- 聯絡專案維護者

## 🔗 相關資源

- [ESP32 官方文檔](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [PlatformIO 文檔](https://docs.platformio.org/)
- [MQTT 協定說明](https://mqtt.org/)
- [Homebridge 外掛開發](https://developers.homebridge.io/)
- [IRremoteESP8266 API](https://crankyoldgit.github.io/IRremoteESP8266/doxygen/html/)

---

⭐ 如果這個專案對您有幫助，請給予 Star 支持！
