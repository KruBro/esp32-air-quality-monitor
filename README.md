---

# 🏠 Indoor Air Quality Monitor — ESP32 Firmware

### Optimized Firmware v6.0

```
██╗███╗   ██╗██████╗  ██████╗  ██████╗ ██████╗ 
██║████╗  ██║██╔══██╗██╔═══██╗██╔════╝ ██╔══██╗
██║██╔██╗ ██║██████╔╝██║   ██║██║  ███╗██████╔╝
██║██║╚██╗██║██╔══██╗██║   ██║██║   ██║██╔══██╗
██║██║ ╚████║██║  ██║╚██████╔╝╚██████╔╝██║  ██║
╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝
```

🎥 **Project Demo Video:**
[https://youtube.com/shorts/vCJ57eRC9Sw?si=mubItcBHRJwJyPQ4](https://youtube.com/shorts/vCJ57eRC9Sw?si=mubItcBHRJwJyPQ4)

---

## ✅ Overview

This repository contains the **v6.0 optimized ESP32 firmware** for an advanced Indoor Air Quality Monitoring system featuring:

* **BME680 Sensor** (Temperature, Humidity, Pressure, VOC)
* **Sharp GP2Y Dust Sensor**
* **SSD1306 OLED Display**
* **High-performance Async Web Server**
* **Secure WebSocket Authentication**
* **OTA firmware updates**
* **mDNS discovery**
* **Optimized performance, memory, and stability**

Everything is fully non-blocking, secure, and optimized for long-term operation.

---

## 📦 Project Structure

```
indoor_air_quality/
│── src/
│   └── main.cpp                 # v6.0 optimized firmware
│── include/
│   ├── secrets.h                # Your credentials (ignored)
│   ├── secrets.h.example        # Template
│   └── token.h                  # WebSocket token definition
│── .gitignore
│── platformio.ini
└── README.md
```

---

## 🚀 Feature Highlights (Firmware v6.0)

✅ Fully optimized BME680 + GP2Y sensor reading
✅ Dust smoothing (EMA) for stability
✅ Real-time WebSocket streaming
✅ Secure bearer-token authentication
✅ OTA update support
✅ mDNS (`air-quality.local`)
✅ Async TCP/WebSocket server
✅ Rate limiting for attackers
✅ Clean system state + watchdog-like resilience
✅ Optimized memory usage
✅ Optimized I2C + ADC performance
✅ OLED live display

---

## 🔧 Hardware Overview

(Same as previous version, kept concise)

* **ESP32 Dev Module**
* **BME680 I2C** (3.3V)
* **Sharp GP2Y Dust Sensor**

  * LED Pin → GPIO 27
  * ADC Output → GPIO 34
* **SSD1306 OLED** (I2C)
* **5V supply recommended for dust sensor**

---

## ✅ Setup Guide

### 1. Clone the repository

```bash
git clone https://github.com/<user>/<repo>.git
cd indoor_air_quality
```

### 2. Install PlatformIO

VS Code → Extensions → PlatformIO IDE

### 3. Create your secrets file

Copy template:

```
include/secrets.h.example → include/secrets.h
```

Edit:

```c
#define WIFI_SSID        "YourWiFi"
#define WIFI_PASSWORD    "Password123"
#define OTA_PASSWORD     "mysuperpassword"
#define DEVICE_HOSTNAME  "air-quality"
```

⚠️ **Do not commit secrets.h**

---

### 4. Configure WebSocket Token

Edit:

```
include/token.h
```

Example:

```c
#define WS_BEARER_TOKEN "your_64byte_secure_token"
```

Your frontend **must** use the same token.

---

### 5. (Optional) Assign Static IP

Used for stable WebSocket + OTA.

Example used in testing:

```
192.168.1.5
```

---

### 6. Build & Upload (first time via USB)

```bash
pio run --target upload
```

---

### 7. OTA Upload (any time ESP32 is online)

```bash
pio run --target upload
```

The firmware already includes:

```ini
upload_protocol = espota
```

---

### ✅ REST API Endpoints

#### `/status`

Requires:

```
Authorization: Bearer <WS_BEARER_TOKEN>
```

Returns system + sensor diagnostics.

#### `/health`

No auth required.
Returns:

```
OK
```

---

## 📊 WebSocket Data Format

ESP32 sends this packet every 2 seconds:

```json
{
  "temp": 24.5,
  "hum": 63.2,
  "pres": 1005.3,
  "gas": 21.03,
  "dust": 180,
  "iaq": 51,
  "ts": 12345678
}
```

---

## 🧪 GP2Y Dust Sensor Notes

* Requires stable **5V**
* Uses an **infrared LED pulse timing cycle**
* ADC reading on GPIO 34 with 11 dB attenuation
* Output depends on:

  * Clean airflow
  * No dust accumulated inside sensor cavity
  * Proper timing (already optimized)

---

## 🛡 Security Summary (Firmware v6.0)

* Secure WebSocket token authentication
* REST API token validation
* OTA password hashing
* Rate limiting per-client
* Private key/material stored in header files but ignored via .gitignore
* No plaintext secrets stored in repo

---

## 📄 License

MIT License.

---

## 🙌 Contributions

PRs, issues, and suggestions are welcome.

---
