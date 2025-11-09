
```markdown
# 🏠 Indoor Air Quality Monitor — ESP32 Firmware  
### Optimized Firmware v6.0

```

██╗███╗   ██╗██████╗  ██████╗  ██████╗ ██████╗
██║████╗  ██║██╔══██╗██╔═══██╗██╔════╝ ██╔══██╗
██║██╔██╗ ██║██████╔╝██║   ██║██║  ███╗██████╔╝
██║██║╚██╗██║██╔══██╗██║   ██║██║   ██║██╔══██╗
██║██║ ╚████║██║  ██║╚██████╔╝╚██████╔╝██║  ██║
╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝

````

🎥 **Project Demo Video:**  
https://youtube.com/shorts/vCJ57eRC9Sw?si=mubItcBHRJwJyPQ4

---

## ✅ Overview  
This firmware powers a high-accuracy indoor air-quality monitoring system using **ESP32**, **BME680**, and **Sharp GP2Y Dust Sensor**, with:  
• Fully optimized sensor sampling  
• WebSocket-based real-time streaming  
• Token-secured communication  
• OLED display output  
• OTA updates  
• mDNS device discovery  

Firmware v6.0 focuses on **stability, speed, and zero performance bottlenecks**.

---

## 🧠 Features  
- BME680 environmental data  
  - Temperature  
  - Humidity  
  - Pressure  
  - Gas resistance  
  - Computed IAQ score  
- GP2Y dust sensor  
  - ADC-smoothed readings  
  - Exponential filtering  
- OLED display live UI  
- WebSocket broadcast every 2 seconds  
- Token-based auth  
- WiFi watchdog auto-repair  
- OTA firmware upgrade  
- REST `/status` health endpoint  
- mDNS hostname: `air-quality.local`

---

## 📡 Endpoints

### WebSocket  
Path: `/ws`  
Authentication:  
```json
{ "type": "auth", "token": "<bearer_token>" }
````

### REST API

`GET /status`
Headers required:

```
Authorization: Bearer <token>
```

---

## 🛠 Hardware Used

* ESP32 Dev Module
* BME680 Sensor
* Sharp GP2Y1010AU0F Dust Sensor
* OLED SSD1306 I2C (128x64)
* 3.3V logic level wiring

---

## 🔧 Build & Flash

1. Install ESP32 board package in Arduino IDE
2. Select:

   * Board: **ESP32 Dev Module**
   * Upload speed: 921600
   * Partition Scheme: “Default (2MB APP / 2MB SPIFFS)”
3. Install required libraries:

   * Adafruit BME680
   * Adafruit SSD1306
   * ESP Async WebServer
   * AsyncTCP
   * ArduinoJson
4. Flash firmware
5. Access device at:

```
http://air-quality.local
```

---

## ♻ OTA Update

Upload new firmware over WiFi after connecting to the device’s network.
OTA password hash is already embedded in code.

---

## 📦 Version

**Firmware v6.0 Stable**
Fully optimized and memory-safe for long-term reliability.

---

```
```
