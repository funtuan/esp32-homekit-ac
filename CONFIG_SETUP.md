# 🔒 安全配置說明

本專案使用配置文件來管理敏感資訊，確保您的 WiFi 密碼、MQTT 伺服器地址等不會被意外上傳到公開儲存庫。

## 📋 快速開始

### 1. 複製配置範本

```bash
cd src
cp config.h.example config.h
```

### 2. 編輯配置文件

使用任何文本編輯器打開 `src/config.h`，填入您的實際配置：

```cpp
// WiFi 設定
#define WIFI_SSID           "您的WiFi名稱"
#define WIFI_PASSWORD       "您的WiFi密碼"

// MQTT 設定
#define MQTT_SERVER         "您的MQTT伺服器IP"  // 例如：192.168.1.100
#define MQTT_PORT           1883
```

### 3. 驗證配置

確認 `config.h` 文件存在且包含正確的配置資訊後，即可編譯並上傳程式：

```bash
pio run -t upload
```

## 🔐 安全注意事項

### ✅ 已被保護的文件

以下文件已加入 `.gitignore`，**不會被上傳**到 Git：
- `src/config.h` - 包含您的實際敏感資訊
- `*.local` - 任何本地配置文件
- `.env` - 環境變數文件

### ⚠️ 需要上傳的文件

以下文件**應該上傳**到 Git，供其他開發者參考：
- `src/config.h.example` - 配置範本（不含真實資料）
- `.gitignore` - Git 忽略規則
- 本說明文件

## 📝 配置項目說明

### WiFi 配置

| 項目 | 說明 | 範例 |
|------|------|------|
| `WIFI_SSID` | WiFi 網路名稱 | `"MyHomeWiFi"` |
| `WIFI_PASSWORD` | WiFi 密碼 | `"mySecurePassword123"` |

### MQTT 配置

| 項目 | 說明 | 範例 |
|------|------|------|
| `MQTT_SERVER` | MQTT Broker IP 地址 | `"192.168.1.100"` |
| `MQTT_PORT` | MQTT 端口（通常 1883） | `1883` |
| `MQTT_CLIENT_ID` | 客戶端識別碼 | `"ESP32_AC_Controller"` |
| `MQTT_USER` | MQTT 用戶名（選填） | `""` 或 `"username"` |
| `MQTT_PASSWORD` | MQTT 密碼（選填） | `""` 或 `"password"` |

### 硬體配置

| 項目 | 說明 | 預設值 |
|------|------|--------|
| `IR_TX_GPIO` | 紅外線發射器 GPIO 引腳 | `4` |
| `LED_BUILTIN` | 內建 LED 引腳 | `2` |

## 🔍 如何獲取 MQTT 伺服器 IP

### macOS
```bash
ifconfig | grep "inet " | grep -v 127.0.0.1
```

### Linux
```bash
hostname -I
```

### Windows
```cmd
ipconfig
```

尋找類似 `192.168.x.x` 的 IP 地址。

## 🚨 常見問題

### Q: 我不小心上傳了 config.h 怎麼辦？

**A:** 立即執行以下步驟：

1. 從 Git 歷史中移除文件：
```bash
git rm --cached src/config.h
git commit -m "Remove sensitive config file"
git push
```

2. **更改您的 WiFi 密碼**（如果已公開）

3. 檢查 GitHub/GitLab 是否還能看到舊版本，必要時聯繫平台支援刪除

### Q: config.h 被刪除了怎麼辦？

**A:** 從範本重新建立：
```bash
cp src/config.h.example src/config.h
# 然後編輯 config.h 填入您的配置
```

### Q: 團隊協作時如何分享配置？

**A:** 
- ✅ **正確做法**: 通過安全管道（如加密訊息）分享配置內容
- ❌ **錯誤做法**: 將 config.h 提交到 Git

### Q: 我可以使用不同的配置文件名嗎？

**A:** 可以，但需要：
1. 修改 `main.cpp` 中的 `#include "config.h"`
2. 在 `.gitignore` 中加入新的文件名

## 📚 相關文件

- [QUICK_START.md](QUICK_START.md) - 快速開始指南
- [README_MQTT.md](README_MQTT.md) - MQTT 設定詳細說明
- [platformio.ini](../platformio.ini) - PlatformIO 配置

## 🛠️ 技術支援

如有問題，請檢查：
1. `config.h` 文件是否存在於 `src/` 目錄
2. 配置內容是否正確（無引號配對錯誤等）
3. WiFi 和 MQTT 伺服器是否可連接

---

**記住：永遠不要將包含真實密碼的文件上傳到公開儲存庫！** 🔒
